 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */

#include "squid-old.h"
#include "comm/Connection.h"
#include "mgr/Registration.h"
#include "Store.h"
#include "pconn.h"
#include "comm.h"
#include "fde.h"

#define PCONN_FDS_SZ	8	/* pconn set size, increase for better memcache hit rate */

//TODO: re-attach to MemPools. WAS: static MemAllocator *pconn_fds_pool = NULL;
PconnModule * PconnModule::instance = NULL;
CBDATA_CLASS_INIT(IdleConnList);

/* ========== IdleConnList ============================================ */

IdleConnList::IdleConnList(const char *key, PconnPool *thePool) :
        capacity_(PCONN_FDS_SZ),
        size_(0),
        parent_(thePool)
{
    hash.key = xstrdup(key);
    theList_ = new Comm::ConnectionPointer[capacity_];
// TODO: re-attach to MemPools. WAS: theList = (?? *)pconn_fds_pool->alloc();
}

IdleConnList::~IdleConnList()
{
    if (parent_)
        parent_->unlinkList(this);

    delete[] theList_;

    xfree(hash.key);
}

/** Search the list. Matches by FD socket number.
 * Performed from the end of list where newest entries are.
 *
 * \retval <0   The connection is not listed
 * \retval >=0  The connection array index
 */
int
IdleConnList::findIndexOf(const Comm::ConnectionPointer &conn) const
{
    for (int index = size_ - 1; index >= 0; --index) {
        if (conn->fd == theList_[index]->fd) {
            debugs(48, 3, HERE << "found " << conn << " at index " << index);
            return index;
        }
    }

    debugs(48, 2, HERE << conn << " NOT FOUND!");
    return -1;
}

/** Remove the entry at specified index.
 * May perform a shuffle of list entries to fill the gap.
 * \retval false The index is not an in-use entry.
 */
bool
IdleConnList::removeAt(int index)
{
    if (index < 0 || index >= size_)
        return false;

    // shuffle the remaining entries to fill the new gap.
    for (; index < size_ - 1; ++index)
        theList_[index] = theList_[index + 1];
    theList_[--size_] = NULL;

    if (parent_) {
        parent_->noteConnectionRemoved();
        if (size_ == 0) {
            debugs(48, 3, HERE << "deleting " << hashKeyStr(&hash));
            delete this;
        }
    }

    return true;
}

// almost a duplicate of removeFD. But drops multiple entries.
void
IdleConnList::closeN(size_t n)
{
    if (n < 1) {
        debugs(48, 2, HERE << "Nothing to do.");
        return;
    } else if (n >= (size_t)size_) {
        debugs(48, 2, HERE << "Closing all entries.");
        while (size_ > 0) {
            const Comm::ConnectionPointer conn = theList_[--size_];
            theList_[size_] = NULL;
            clearHandlers(conn);
            conn->close();
            if (parent_)
                parent_->noteConnectionRemoved();
        }
    } else { //if (n < size_)
        debugs(48, 2, HERE << "Closing " << n << " of " << size_ << " entries.");

        size_t index;
        // ensure the first N entries are closed
        for (index = 0; index < n; ++index) {
            const Comm::ConnectionPointer conn = theList_[index];
            theList_[index] = NULL;
            clearHandlers(conn);
            conn->close();
            if (parent_)
                parent_->noteConnectionRemoved();
        }
        // shuffle the list N down.
        for (index = 0; index < (size_t)size_ - n; ++index) {
            theList_[index] = theList_[index + n];
        }
        // ensure the last N entries are unset
        while (index < ((size_t)size_)) {
            theList_[index] = NULL;
            ++index;
        }
        size_ -= n;
    }

    if (parent_ && size_ == 0) {
        debugs(48, 3, HERE << "deleting " << hashKeyStr(&hash));
        delete this;
    }
}

void
IdleConnList::clearHandlers(const Comm::ConnectionPointer &conn)
{
    debugs(48, 3, HERE << "removing close handler for " << conn);
    comm_read_cancel(conn->fd, IdleConnList::Read, this);
    commUnsetConnTimeout(conn);
}

void
IdleConnList::push(const Comm::ConnectionPointer &conn)
{
    if (size_ == capacity_) {
        debugs(48, 3, HERE << "growing idle Connection array");
        capacity_ <<= 1;
        const Comm::ConnectionPointer *oldList = theList_;
        theList_ = new Comm::ConnectionPointer[capacity_];
        for (int index = 0; index < size_; ++index)
            theList_[index] = oldList[index];

        delete[] oldList;
    }

    if (parent_)
        parent_->noteConnectionAdded();

    theList_[size_] = conn;
    ++size_;
    AsyncCall::Pointer readCall = commCbCall(5,4, "IdleConnList::Read",
                                  CommIoCbPtrFun(IdleConnList::Read, this));
    comm_read(conn, fakeReadBuf_, sizeof(fakeReadBuf_), readCall);
    AsyncCall::Pointer timeoutCall = commCbCall(5,4, "IdleConnList::Timeout",
                                     CommTimeoutCbPtrFun(IdleConnList::Timeout, this));
    commSetConnTimeout(conn, Config.Timeout.serverIdlePconn, timeoutCall);
}

/// Determine whether an entry in the idle list is available for use.
/// Returns false if the entry is unset, closed or closing.
bool
IdleConnList::isAvailable(int i) const
{
    const Comm::ConnectionPointer &conn = theList_[i];

    // connection already closed. useless.
    if (!Comm::IsConnOpen(conn))
        return false;

    // our connection early-read/close handler is scheduled to run already. unsafe
    if (!COMMIO_FD_READCB(conn->fd)->active())
        return false;

    return true;
}

Comm::ConnectionPointer
IdleConnList::pop()
{
    for (int i=size_-1; i>=0; --i) {

        if (!isAvailable(i))
            continue;

        // our connection timeout handler is scheduled to run already. unsafe for now.
        // TODO: cancel the pending timeout callback and allow re-use of the conn.
        if (fd_table[theList_[i]->fd].timeoutHandler == NULL)
            continue;

        // finally, a match. pop and return it.
        Comm::ConnectionPointer result = theList_[i];
        /* may delete this */
        removeAt(i);
        clearHandlers(result);
        return result;
    }

    return Comm::ConnectionPointer();
}

/*
 * XXX this routine isn't terribly efficient - if there's a pending
 * read event (which signifies the fd will close in the next IO loop!)
 * we ignore the FD and move onto the next one. This means, as an example,
 * if we have a lot of FDs open to a very popular server and we get a bunch
 * of requests JUST as they timeout (say, it shuts down) we'll be wasting
 * quite a bit of CPU. Just keep it in mind.
 */
Comm::ConnectionPointer
IdleConnList::findUseable(const Comm::ConnectionPointer &key)
{
    assert(size_);

    // small optimization: do the constant bool tests only once.
    const bool keyCheckAddr = !key->local.IsAnyAddr();
    const bool keyCheckPort = key->local.GetPort() > 0;

    for (int i=size_-1; i>=0; --i) {

        if (!isAvailable(i))
            continue;

        // local end port is required, but dont match.
        if (keyCheckPort && key->local.GetPort() != theList_[i]->local.GetPort())
            continue;

        // local address is required, but does not match.
        if (keyCheckAddr && key->local.matchIPAddr(theList_[i]->local) != 0)
            continue;

        // our connection timeout handler is scheduled to run already. unsafe for now.
        // TODO: cancel the pending timeout callback and allow re-use of the conn.
        if (fd_table[theList_[i]->fd].timeoutHandler == NULL)
            continue;

        // finally, a match. pop and return it.
        Comm::ConnectionPointer result = theList_[i];
        /* may delete this */
        removeAt(i);
        clearHandlers(result);
        return result;
    }

    return Comm::ConnectionPointer();
}

/* might delete list */
void
IdleConnList::findAndClose(const Comm::ConnectionPointer &conn)
{
    const int index = findIndexOf(conn);
    if (index >= 0) {
        /* might delete this */
        removeAt(index);
        clearHandlers(conn);
        conn->close();
    }
}

void
IdleConnList::Read(const Comm::ConnectionPointer &conn, char *buf, size_t len, comm_err_t flag, int xerrno, void *data)
{
    debugs(48, 3, HERE << len << " bytes from " << conn);

    if (flag == COMM_ERR_CLOSING) {
        debugs(48, 3, HERE << "COMM_ERR_CLOSING from " << conn);
        /* Bail out on COMM_ERR_CLOSING - may happen when shutdown aborts our idle FD */
        return;
    }

    IdleConnList *list = (IdleConnList *) data;
    /* may delete list/data */
    list->findAndClose(conn);
}

void
IdleConnList::Timeout(const CommTimeoutCbParams &io)
{
    debugs(48, 3, HERE << io.conn);
    IdleConnList *list = static_cast<IdleConnList *>(io.data);
    /* may delete list/data */
    list->findAndClose(io.conn);
}

/* ========== PconnPool PRIVATE FUNCTIONS ============================================ */

const char *
PconnPool::key(const Comm::ConnectionPointer &destLink, const char *domain)
{
    LOCAL_ARRAY(char, buf, SQUIDHOSTNAMELEN * 3 + 10);

    destLink->remote.ToURL(buf, SQUIDHOSTNAMELEN * 3 + 10);
    if (domain) {
        const int used = strlen(buf);
        snprintf(buf+used, SQUIDHOSTNAMELEN * 3 + 10-used, "/%s", domain);
    }

    debugs(48,6,"PconnPool::key(" << destLink << ", " << (domain?domain:"[no domain]") << ") is {" << buf << "}" );
    return buf;
}

void
PconnPool::dumpHist(StoreEntry * e) const
{
    storeAppendPrintf(e,
                      "%s persistent connection counts:\n"
                      "\n"
                      "\treq/\n"
                      "\tconn      count\n"
                      "\t----  ---------\n",
                      descr);

    for (int i = 0; i < PCONN_HIST_SZ; ++i) {
        if (hist[i] == 0)
            continue;

        storeAppendPrintf(e, "\t%4d  %9d\n", i, hist[i]);
    }
}

void
PconnPool::dumpHash(StoreEntry *e) const
{
    hash_table *hid = table;
    hash_first(hid);

    int i = 0;
    for (hash_link *walker = hid->next; walker; walker = hash_next(hid)) {
        storeAppendPrintf(e, "\t item %5d: %s\n", i, (char *)(walker->key));
        ++i;
    }
}

/* ========== PconnPool PUBLIC FUNCTIONS ============================================ */

PconnPool::PconnPool(const char *aDescr) : table(NULL), descr(aDescr),
        theCount(0)
{
    int i;
    table = hash_create((HASHCMP *) strcmp, 229, hash_string);

    for (i = 0; i < PCONN_HIST_SZ; ++i)
        hist[i] = 0;

    PconnModule::GetInstance()->add(this);
}

PconnPool::~PconnPool()
{
    descr = NULL;
    hashFreeMemory(table);
}

void
PconnPool::push(const Comm::ConnectionPointer &conn, const char *domain)
{
    if (fdUsageHigh()) {
        debugs(48, 3, HERE << "Not many unused FDs");
        conn->close();
        return;
    } else if (shutting_down) {
        conn->close();
        debugs(48, 3, HERE << "Squid is shutting down. Refusing to do anything");
        return;
    }

    const char *aKey = key(conn, domain);
    IdleConnList *list = (IdleConnList *) hash_lookup(table, aKey);

    if (list == NULL) {
        list = new IdleConnList(aKey, this);
        debugs(48, 3, HERE << "new IdleConnList for {" << hashKeyStr(&list->hash) << "}" );
        hash_join(table, &list->hash);
    } else {
        debugs(48, 3, HERE << "found IdleConnList for {" << hashKeyStr(&list->hash) << "}" );
    }

    list->push(conn);
    assert(!comm_has_incomplete_write(conn->fd));

    LOCAL_ARRAY(char, desc, FD_DESC_SZ);
    snprintf(desc, FD_DESC_SZ, "Idle server: %s", aKey);
    fd_note(conn->fd, desc);
    debugs(48, 3, HERE << "pushed " << conn << " for " << aKey);
}

Comm::ConnectionPointer
PconnPool::pop(const Comm::ConnectionPointer &destLink, const char *domain, bool isRetriable)
{
    const char * aKey = key(destLink, domain);

    IdleConnList *list = (IdleConnList *)hash_lookup(table, aKey);
    if (list == NULL) {
        debugs(48, 3, HERE << "lookup for key {" << aKey << "} failed.");
        return Comm::ConnectionPointer();
    } else {
        debugs(48, 3, HERE << "found " << hashKeyStr(&list->hash) << (isRetriable?"(to use)":"(to kill)") );
    }

    /* may delete list */
    Comm::ConnectionPointer temp = list->findUseable(destLink);
    if (!isRetriable && Comm::IsConnOpen(temp))
        temp->close();

    return temp;
}

void
PconnPool::closeN(int n, const Comm::ConnectionPointer &destLink, const char *domain)
{
    // TODO: optimize: we can probably do hash_lookup just once
    for (int i = 0; i < n; ++i)
        pop(destLink, domain, false); // may fail!
}

void
PconnPool::unlinkList(IdleConnList *list)
{
    theCount -= list->count();
    assert(theCount >= 0);
    hash_remove_link(table, &list->hash);
}

void
PconnPool::noteUses(int uses)
{
    if (uses >= PCONN_HIST_SZ)
        uses = PCONN_HIST_SZ - 1;

    ++hist[uses];
}

/* ========== PconnModule ============================================ */

/*
 * This simple class exists only for the cache manager
 */

PconnModule::PconnModule() : pools(NULL), poolCount(0)
{
    pools = (PconnPool **) xcalloc(MAX_NUM_PCONN_POOLS, sizeof(*pools));
//TODO: re-link to MemPools. WAS:    pconn_fds_pool = memPoolCreate("pconn_fds", PCONN_FDS_SZ * sizeof(int));
    debugs(48, 0, "persistent connection module initialized");
    registerWithCacheManager();
}

PconnModule *
PconnModule::GetInstance()
