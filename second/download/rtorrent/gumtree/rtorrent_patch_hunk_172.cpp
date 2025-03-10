 
 #include "canvas.h"
 #include "window_title.h"
 
 namespace display {
 
-WindowTitle::WindowTitle(const std::string& s) :
-  Window(new Canvas, false, 1),
-  m_title(s) {
-}
-
 void
 WindowTitle::redraw() {
   m_slotSchedule(this, (cachedTime + rak::timer::from_seconds(1)).round_seconds());
   m_canvas->erase();
 
-  m_canvas->print(std::max(0, (m_canvas->get_width() - (int)m_title.size()) / 2 - 4), 0,
+  m_canvas->print(std::max(0, ((int)m_canvas->width() - (int)m_title.size()) / 2 - 4), 0,
 		  "*** %s ***", m_title.c_str());
 }
 
 }
