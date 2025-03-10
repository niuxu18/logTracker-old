    break;
  }

  return torrent::Object();
}

torrent::Object
apply_d_delete_tied(core::Download* download) {
  const std::string& tie = rpc::call_command_string("d.tied_to_file", rpc::make_target(download));

  if (tie.empty())
    return torrent::Object();

  if (::unlink(rak::path_expand(tie).c_str()) == -1)
    control->core()->push_log_std("Could not unlink tied file: " + std::string(rak::error_number::current().c_str()));

  rpc::call_command("d.tied_to_file.set", std::string(), rpc::make_target(download));
  return torrent::Object();
}

torrent::Object
apply_d_connection_type(core::Download* download, const std::string& name) {
  torrent::Download::ConnectionType connType;

  if (name == "leech")
    connType = torrent::Download::CONNECTION_LEECH;
  else if (name == "seed")
