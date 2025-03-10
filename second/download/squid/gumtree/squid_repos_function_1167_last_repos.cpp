bool
StoreEntry::mayStartSwapOut()
{
    // must be checked in the caller
    assert(!EBIT_TEST(flags, ENTRY_ABORTED));
    assert(!swappingOut());

    if (!Config.cacheSwap.n_configured)
        return false;

    assert(mem_obj);
    const MemObject::SwapOut::Decision &decision = mem_obj->swapout.decision;

    // if we decided that starting is not possible, do not repeat same checks
    if (decision == MemObject::SwapOut::swImpossible) {
        debugs(20, 3, HERE << " already rejected");
        return false;
    }

    // if we swapped out already, do not start over
    if (swap_status == SWAPOUT_DONE) {
        debugs(20, 3, "already did");
        swapOutDecision(MemObject::SwapOut::swImpossible);
        return false;
    }

    // if we stared swapping out already, do not start over
    if (decision == MemObject::SwapOut::swStarted) {
        debugs(20, 3, "already started");
        swapOutDecision(MemObject::SwapOut::swImpossible);
        return false;
    }

    // if we decided that swapout is possible, do not repeat same checks
    if (decision == MemObject::SwapOut::swPossible) {
        debugs(20, 3, "already allowed");
        return true;
    }

    if (!checkCachable()) {
        debugs(20, 3,  HERE << "not cachable");
        swapOutDecision(MemObject::SwapOut::swImpossible);
        return false;
    }

    if (EBIT_TEST(flags, ENTRY_SPECIAL)) {
        debugs(20, 3,  HERE  << url() << " SPECIAL");
        swapOutDecision(MemObject::SwapOut::swImpossible);
        return false;
    }

    if (mem_obj->inmem_lo > 0) {
        debugs(20, 3, "storeSwapOut: (inmem_lo > 0)  imem_lo:" <<  mem_obj->inmem_lo);
        swapOutDecision(MemObject::SwapOut::swImpossible);
        return false;
    }

    if (!mem_obj->isContiguous()) {
        debugs(20, 3, "storeSwapOut: not Contiguous");
        swapOutDecision(MemObject::SwapOut::swImpossible);
        return false;
    }

    // handle store_maxobjsize limit
    {
        // TODO: add estimated store metadata size to be conservative

        // use guaranteed maximum if it is known
        const int64_t expectedEnd = mem_obj->expectedReplySize();
        debugs(20, 7,  HERE << "expectedEnd = " << expectedEnd);
        if (expectedEnd > store_maxobjsize) {
            debugs(20, 3,  HERE << "will not fit: " << expectedEnd <<
                   " > " << store_maxobjsize);
            swapOutDecision(MemObject::SwapOut::swImpossible);
            return false; // known to outgrow the limit eventually
        }

        // use current minimum (always known)
        const int64_t currentEnd = mem_obj->endOffset();
        if (currentEnd > store_maxobjsize) {
            debugs(20, 3,  HERE << "does not fit: " << currentEnd <<
                   " > " << store_maxobjsize);
            swapOutDecision(MemObject::SwapOut::swImpossible);
            return false; // already does not fit and may only get bigger
        }

        // prevent final default swPossible answer for yet unknown length
        if (expectedEnd < 0 && store_status != STORE_OK) {
            const int64_t more = Store::Root().accumulateMore(*this);
            if (more > 0) {
                debugs(20, 5, "got " << currentEnd << "; defer decision for " << more << " more bytes");
                return true; // may still fit, but no final decision yet
            }
        }
    }

    swapOutDecision(MemObject::SwapOut::swPossible);
    return true;
}