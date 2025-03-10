  utils::displayScheduler.insert(&m_taskUpdate, utils::Timer::cache().round_seconds() + 1000000);
  m_canvas->erase();

  int x = 2;
  int y = 0;

  m_canvas->print(x, y, "DNS");     x += 16;
  m_canvas->print(x, y, "UP");      x += 7;
  m_canvas->print(x, y, "DOWN");    x += 7;
  m_canvas->print(x, y, "PEER");    x += 7;
  m_canvas->print(x, y, "C/RE/LO"); x += 9;
  m_canvas->print(x, y, "QS");      x += 6;
  m_canvas->print(x, y, "DONE");    x += 6;
  m_canvas->print(x, y, "REQ");     x += 6;
  m_canvas->print(x, y, "SNUB");

  ++y;

  if (m_list->empty())
    return;
