   typedef std::pair<unsigned int, unsigned int> Range;
 
   unsigned int group = 0;
   Range range = rak::advance_bidirectional<unsigned int>(0, *m_focus, tl->size(), (m_canvas->height() + 1) / 2);
 
   while (range.first != range.second) {
-    torrent::Tracker tracker = tl->get(range.first);
+    torrent::Tracker* tracker = tl->at(range.first);
 
 //     m_canvas->print(0, pos, "[%c] [S/L %5i/%5i] %s",
-//                     tracker.is_enabled() ? (tracker.is_open() ? '*' : ' ') : '-',
-//                     tracker.scrape_complete(), tracker.scrape_incomplete(),
-//                     tracker.url().c_str());
+//                     tracker->is_enabled() ? (tracker->is_open() ? '*' : ' ') : '-',
+//                     tracker->scrape_complete(), tracker->scrape_incomplete(),
+//                     tracker->url().c_str());
 
-    if (tracker.group() == group)
+    if (tracker->group() == group)
       m_canvas->print(0, pos, "%2i:", group++);
 
     m_canvas->print(4, pos++, "%s",
-                    tracker.url().c_str());
+                    tracker->url().c_str());
 
     m_canvas->print(4, pos++, "Id: %s Focus: %s Enabled: %s Open: %s S/L: %u/%u",
-                    rak::copy_escape_html(tracker.tracker_id()).c_str(),
-                    range.first == tl->focus() ? "yes" : " no",
-                    tracker.is_enabled() ? "yes" : " no",
-                    tracker.is_open() ? "yes" : " no",
-                    tracker.scrape_complete(),
-                    tracker.scrape_incomplete());
+                    rak::copy_escape_html(tracker->tracker_id()).c_str(),
+                    range.first == tl->focus_index() ? "yes" : " no",
+                    tracker->is_usable() ? "yes" : tracker->is_enabled() ? "off" : " no",
+                    tracker->is_busy() ? "yes" : " no",
+                    tracker->scrape_complete(),
+                    tracker->scrape_incomplete());
 
 //     m_canvas->print(4, pos++, "Id: %s Focus: %s Enabled: %s Open: %s Timer: %u/%u",
-//                     rak::copy_escape_html(tracker.tracker_id()).c_str(),
+//                     rak::copy_escape_html(tracker->tracker_id()).c_str(),
 //                     range.first == tl->focus() ? "yes" : " no",
-//                     tracker.is_enabled() ? "yes" : " no",
-//                     tracker.is_open() ? "yes" : " no",
-//                     tracker.normal_interval(),
-//                     tracker.min_interval());
+//                     tracker->is_enabled() ? "yes" : " no",
+//                     tracker->is_open() ? "yes" : " no",
+//                     tracker->normal_interval(),
+//                     tracker->min_interval());
 
     if (range.first == *m_focus) {
       m_canvas->set_attr(0, pos - 2, m_canvas->width(), is_focused() ? A_REVERSE : A_BOLD, COLOR_PAIR(0));
       m_canvas->set_attr(0, pos - 1, m_canvas->width(), is_focused() ? A_REVERSE : A_BOLD, COLOR_PAIR(0));
     }
 
