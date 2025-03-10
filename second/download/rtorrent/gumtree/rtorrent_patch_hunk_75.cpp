 
 void
 WindowFileList::redraw() {
   utils::displayScheduler.insert(&m_taskUpdate, (utils::Timer::cache() + 10 * 1000000).round_seconds());
   m_canvas->erase();
 
-  if (m_download->get_download().get_entry_size() == 0 ||
+  if (m_download->get_download().size_file_entries() == 0 ||
       m_canvas->get_height() < 2)
     return;
 
   int pos = 0;
 
   m_canvas->print( 2, pos, "File");
   m_canvas->print(55, pos, "Size");
   m_canvas->print(63, pos, "Pri");
   m_canvas->print(68, pos, "Cmpl");
+  m_canvas->print(74, pos, "Encoding");
+  m_canvas->print(84, pos, "Chunks");
 
   ++pos;
 
-  if (*m_focus >= m_download->get_download().get_entry_size())
+  if (*m_focus >= m_download->get_download().size_file_entries())
     throw std::logic_error("WindowFileList::redraw() called on an object with a bad focus value");
 
   Range range = rak::advance_bidirectional<unsigned int>(0,
 							 *m_focus,
-							 m_download->get_download().get_entry_size(),
+							 m_download->get_download().size_file_entries(),
 							 m_canvas->get_height() - pos);
 
   while (range.first != range.second) {
-    torrent::Entry e = m_download->get_download().get_entry(range.first);
+    torrent::Entry e = m_download->get_download().file_entry(range.first);
 
-    std::string path = e.get_path();
+    std::string path = e.path_str();
 
     if (path.length() <= 50)
       path = path + std::string(50 - path.length(), ' ');
     else
       path = path.substr(0, 50);
 
     std::string priority;
 
-    switch (e.get_priority()) {
-    case torrent::Entry::STOPPED:
+    switch (e.priority()) {
+    case torrent::Entry::OFF:
       priority = "off";
       break;
 
     case torrent::Entry::NORMAL:
       priority = "   ";
       break;
