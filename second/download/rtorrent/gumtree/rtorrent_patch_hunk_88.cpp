   m_focus(focus) {
 }
 
 void
 WindowTrackerList::redraw() {
   // TODO: Make this depend on tracker signal.
-  utils::displayScheduler.insert(&m_taskUpdate, (utils::Timer::cache() + 10 * 1000000).round_seconds());
+  priority_queue_insert(&displayScheduler, &m_taskUpdate, (cachedTime + 10 * 1000000).round_seconds());
   m_canvas->erase();
 
   int pos = 0;
 
   m_canvas->print( 2, pos, "Trackers:");
 
