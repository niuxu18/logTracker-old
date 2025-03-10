  element->set_column(1);
  element->set_interval(1);

  element->push_back("Peer info:");

  element->push_back("");
  element->push_column("Address:",   te_command("cat=$p.get_address=,:,$p.get_port="));
  element->push_column("Id:",        te_command("p.get_id_html="));
  element->push_column("Client:",    te_command("p.get_client_version="));
  element->push_column("Options:",   te_command("p.get_options_str="));
  element->push_column("Connected:", te_command("if=$p.is_incoming=,incoming,outgoing"));
  element->push_column("Encrypted:", te_command("if=$p.is_encrypted=,yes,$p.is_obfuscated=,handshake,no"));

  element->push_back("");
  element->push_column("Snubbed:",   te_command("if=$p.is_snubbed=,yes,no"));
  element->push_column("Done:",      te_command("p.get_completed_percent="));
  element->push_column("Rate:",      te_command("cat=$to_kb=$p.get_up_rate=,\\ KB\\ ,$to_kb=$p.get_down_rate=,\\ KB"));
  element->push_column("Total:",     te_command("cat=$to_kb=$p.get_up_total=,\\ KB\\ ,$to_kb=$p.get_down_total=,\\ KB"));

  element->set_column_width(element->column_width() + 1);
  element->set_error_handler(new display::TextElementCString("No peer selected."));

  return element;
}
