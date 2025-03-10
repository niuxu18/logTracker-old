static int refresh_lists(void) {
  int n;

  n = virConnectNumOfDomains(conn);
  if (n < 0) {
    VIRT_ERROR(conn, "reading number of domains");
    return -1;
  }

  if (n > 0) {
    int *domids;

    /* Get list of domains. */
    domids = malloc(sizeof(*domids) * n);
    if (domids == NULL) {
      ERROR(PLUGIN_NAME " plugin: malloc failed.");
      return -1;
    }

    n = virConnectListDomains(conn, domids, n);
    if (n < 0) {
      VIRT_ERROR(conn, "reading list of domains");
      sfree(domids);
      return -1;
    }

    free_block_devices();
    free_interface_devices();
    free_domains();

    /* Fetch each domain and add it to the list, unless ignore. */
    for (int i = 0; i < n; ++i) {
      virDomainPtr dom = NULL;
      const char *name;
      char *xml = NULL;
      xmlDocPtr xml_doc = NULL;
      xmlXPathContextPtr xpath_ctx = NULL;
      xmlXPathObjectPtr xpath_obj = NULL;

      dom = virDomainLookupByID(conn, domids[i]);
      if (dom == NULL) {
        VIRT_ERROR(conn, "virDomainLookupByID");
        /* Could be that the domain went away -- ignore it anyway. */
        continue;
      }

      name = virDomainGetName(dom);
      if (name == NULL) {
        VIRT_ERROR(conn, "virDomainGetName");
        goto cont;
      }

      if (il_domains && ignorelist_match(il_domains, name) != 0)
        goto cont;

      if (add_domain(dom) < 0) {
        ERROR(PLUGIN_NAME " plugin: malloc failed.");
        goto cont;
      }

      /* Get a list of devices for this domain. */
      xml = virDomainGetXMLDesc(dom, 0);
      if (!xml) {
        VIRT_ERROR(conn, "virDomainGetXMLDesc");
        goto cont;
      }

      /* Yuck, XML.  Parse out the devices. */
      xml_doc = xmlReadDoc((xmlChar *)xml, NULL, NULL, XML_PARSE_NONET);
      if (xml_doc == NULL) {
        VIRT_ERROR(conn, "xmlReadDoc");
        goto cont;
      }

      xpath_ctx = xmlXPathNewContext(xml_doc);

      /* Block devices. */
      xpath_obj = xmlXPathEval((xmlChar *)"/domain/devices/disk/target[@dev]",
                               xpath_ctx);
      if (xpath_obj == NULL || xpath_obj->type != XPATH_NODESET ||
          xpath_obj->nodesetval == NULL)
        goto cont;

      for (int j = 0; j < xpath_obj->nodesetval->nodeNr; ++j) {
        xmlNodePtr node;
        char *path = NULL;

        node = xpath_obj->nodesetval->nodeTab[j];
        if (!node)
          continue;
        path = (char *)xmlGetProp(node, (xmlChar *)"dev");
        if (!path)
          continue;

        if (il_block_devices &&
            ignore_device_match(il_block_devices, name, path) != 0)
          goto cont2;

        add_block_device(dom, path);
      cont2:
        if (path)
          xmlFree(path);
      }
      xmlXPathFreeObject(xpath_obj);

      /* Network interfaces. */
      xpath_obj = xmlXPathEval(
          (xmlChar *)"/domain/devices/interface[target[@dev]]", xpath_ctx);
      if (xpath_obj == NULL || xpath_obj->type != XPATH_NODESET ||
          xpath_obj->nodesetval == NULL)
        goto cont;

      xmlNodeSetPtr xml_interfaces = xpath_obj->nodesetval;

      for (int j = 0; j < xml_interfaces->nodeNr; ++j) {
        char *path = NULL;
        char *address = NULL;
        xmlNodePtr xml_interface;

        xml_interface = xml_interfaces->nodeTab[j];
        if (!xml_interface)
          continue;

        for (xmlNodePtr child = xml_interface->children; child;
             child = child->next) {
          if (child->type != XML_ELEMENT_NODE)
            continue;

          if (xmlStrEqual(child->name, (const xmlChar *)"target")) {
            path = (char *)xmlGetProp(child, (const xmlChar *)"dev");
            if (!path)
              continue;
          } else if (xmlStrEqual(child->name, (const xmlChar *)"mac")) {
            address = (char *)xmlGetProp(child, (const xmlChar *)"address");
            if (!address)
              continue;
          }
        }

        if (il_interface_devices &&
            (ignore_device_match(il_interface_devices, name, path) != 0 ||
             ignore_device_match(il_interface_devices, name, address) != 0))
          goto cont3;

        add_interface_device(dom, path, address, j + 1);
      cont3:
        if (path)
          xmlFree(path);
        if (address)
          xmlFree(address);
      }

    cont:
      if (xpath_obj)
        xmlXPathFreeObject(xpath_obj);
      if (xpath_ctx)
        xmlXPathFreeContext(xpath_ctx);
      if (xml_doc)
        xmlFreeDoc(xml_doc);
      sfree(xml);
    }

    sfree(domids);
  }

  return 0;
}