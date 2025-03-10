     debugs(20, 3, HERE << "mem-loading failed; freeing " << index);
     map->freeEntry(index); // do not let others into the same trap
     return NULL;
 }
 
 void
-MemStore::get(String const key, STOREGETCLIENT aCallback, void *aCallbackData)
+MemStore::updateHeaders(StoreEntry *updatedE)
 {
-    // XXX: not needed but Store parent forces us to implement this
-    fatal("MemStore::get(key,callback,data) should not be called");
+    if (!map)
+        return;
+
+    Ipc::StoreMapUpdate update(updatedE);
+    assert(updatedE);
+    assert(updatedE->mem_obj);
+    if (!map->openForUpdating(update, updatedE->mem_obj->memCache.index))
+        return;
+
+    try {
+        updateHeadersOrThrow(update);
+    } catch (const std::exception &ex) {
+        debugs(20, 2, "error starting to update entry " << *updatedE << ": " << ex.what());
+        map->abortUpdating(update);
+    }
+}
+
+void
+MemStore::updateHeadersOrThrow(Ipc::StoreMapUpdate &update)
+{
+    // our +/- hdr_sz math below does not work if the chains differ [in size]
+    Must(update.stale.anchor->basics.swap_file_sz == update.fresh.anchor->basics.swap_file_sz);
+
+    const HttpReply *rawReply = update.entry->getReply();
+    Must(rawReply);
+    const HttpReply &reply = *rawReply;
+    const uint64_t staleHdrSz = reply.hdr_sz;
+    debugs(20, 7, "stale hdr_sz: " << staleHdrSz);
+
+    /* we will need to copy same-slice payload after the stored headers later */
+    Must(staleHdrSz > 0);
+    update.stale.splicingPoint = map->sliceContaining(update.stale.fileNo, staleHdrSz);
+    Must(update.stale.splicingPoint >= 0);
+    Must(update.stale.anchor->basics.swap_file_sz >= staleHdrSz);
+
+    Must(update.stale.anchor);
+    ShmWriter writer(*this, update.entry, update.fresh.fileNo);
+    reply.packHeadersInto(&writer);
+    const uint64_t freshHdrSz = writer.totalWritten;
+    debugs(20, 7, "fresh hdr_sz: " << freshHdrSz << " diff: " << (freshHdrSz - staleHdrSz));
+
+    /* copy same-slice payload remaining after the stored headers */
+    const Ipc::StoreMapSlice &slice = map->readableSlice(update.stale.fileNo, update.stale.splicingPoint);
+    const Ipc::StoreMapSlice::Size sliceCapacity = Ipc::Mem::PageSize();
+    const Ipc::StoreMapSlice::Size headersInLastSlice = staleHdrSz % sliceCapacity;
+    Must(headersInLastSlice > 0); // or sliceContaining() would have stopped earlier
+    Must(slice.size >= headersInLastSlice);
+    const Ipc::StoreMapSlice::Size payloadInLastSlice = slice.size - headersInLastSlice;
+    const MemStoreMapExtras::Item &extra = extras->items[update.stale.splicingPoint];
+    char *page = static_cast<char*>(PagePointer(extra.page));
+    debugs(20, 5, "appending same-slice payload: " << payloadInLastSlice);
+    writer.append(page + headersInLastSlice, payloadInLastSlice);
+    update.fresh.splicingPoint = writer.lastSlice;
+
+    update.fresh.anchor->basics.swap_file_sz -= staleHdrSz;
+    update.fresh.anchor->basics.swap_file_sz += freshHdrSz;
+
+    map->closeForUpdating(update);
 }
 
 bool
 MemStore::anchorCollapsed(StoreEntry &collapsed, bool &inSync)
 {
     if (!map)
