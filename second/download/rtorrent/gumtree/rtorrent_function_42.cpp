void
WindowPeerList::redraw() {
  m_nextDraw = utils::Timer::cache().round_seconds() + 1000000;
  m_canvas->erase();

  int x = 2;
  int y = 0;

  m_canvas->print(x, y, "DNS");   x += 16;
  m_canvas->print(x, y, "UP");    x += 7;
  m_canvas->print(x, y, "DOWN");  x += 7;
  m_canvas->print(x, y, "PEER");  x += 7;
  m_canvas->print(x, y, "RE/LO"); x += 7;
  m_canvas->print(x, y, "QS");    x += 6;
  m_canvas->print(x, y, "DONE");  x += 6;
  m_canvas->print(x, y, "REQ");   x += 6;
  m_canvas->print(x, y, "SNUB");

  ++y;

  if (m_list->empty())
    return;

  typedef std::pair<PList::iterator, PList::iterator> Range;

  Range range = rak::advance_bidirectional(m_list->begin(),
					   *m_focus != m_list->end() ? *m_focus : m_list->begin(),
					   m_list->end(),
					   m_canvas->get_height() - y);

  if (m_download->get_download().get_chunks_total() <= 0)
    throw std::logic_error("WindowPeerList::redraw() m_slotChunksTotal() returned invalid value");

  while (range.first != range.second) {
    torrent::Peer& p = *range.first;

    x = 0;

    m_canvas->print(x, y, "%c %s",
		    range.first == *m_focus ? '*' : ' ',
		    p.get_dns().c_str());
    x += 18;

    m_canvas->print(x, y, "%.1f", (double)p.get_rate_up() / 1024);
    x += 7;

    m_canvas->print(x, y, "%.1f", (double)p.get_rate_down() / 1024);
    x += 7;

    m_canvas->print(x, y, "%.1f", (double)p.get_rate_peer() / 1024);
    x += 7;

    m_canvas->print(x, y, "%c%c/%c%c%c",
		    p.get_remote_choked() ? 'c' : 'u',
		    p.get_remote_interested() ? 'i' : 'n',
		    p.get_local_choked() ? 'c' : 'u',
		    p.get_local_interested() ? 'i' : 'n',
		    p.get_choke_delayed() ? 'd' : ' ');
    x += 7;

    m_canvas->print(x, y, "%i/%i",
		    p.get_outgoing_queue_size(),
		    p.get_incoming_queue_size());
    x += 6;

    m_canvas->print(x, y, "%3i", done_percentage(*range.first));
    x += 6;

    if (p.get_incoming_queue_size())
      m_canvas->print(x, y, "%i", p.get_incoming_index(0));

    x += 6;

    if (p.get_snubbed())
      m_canvas->print(x, y, "*");

    ++y;
    ++range.first;
  }
}