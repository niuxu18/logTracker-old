  Window(new Canvas, false, 3),
  m_download(d) {
}

void
WindowDownloadStatusbar::redraw() {
  if (utils::Timer::cache() - m_lastDraw < 1000000)
    return;

  m_lastDraw = utils::Timer::cache();

  m_canvas->erase();

  if (m_download->get_download().get_chunks_done() != m_download->get_download().get_chunks_total() || !m_download->get_download().is_open())
    m_canvas->print(0, 0, "Torrent: %.1f / %.1f MiB Rate:%5.1f /%5.1f KiB Uploaded: %.1f MiB",
		    (double)m_download->get_download().get_bytes_done() / (double)(1 << 20),
