char*
print_status_extra(char* first, char* last) {
  first = print_buffer(first, last, " [U %i/%i]",
                       torrent::currently_unchoked(),
                       torrent::max_unchoked());

  first = print_buffer(first, last, " [D %i/%i]",
                       torrent::download_unchoked(),
                       torrent::max_download_unchoked());

  first = print_buffer(first, last, " [H %u/%u]",
                       control->core()->get_poll_manager()->get_http_stack()->active(),
                       control->core()->get_poll_manager()->get_http_stack()->max_active());                       

  first = print_buffer(first, last, " [S %i/%i/%i]",
                       torrent::total_handshakes(),
