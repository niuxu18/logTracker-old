static void
build_node (XML_Parser parser,
	    int src_node,
	    XML_Content *dest,
	    XML_Content **contpos,
	    char **strpos)
{
  dest->type = dtd.scaffold[src_node].type;
  dest->quant = dtd.scaffold[src_node].quant;
  if (dest->type == XML_CTYPE_NAME) {
    const char *src;
    dest->name = *strpos;
    src = dtd.scaffold[src_node].name;
    for (;;) {
      *(*strpos)++ = *src;
      if (! *src)
	break;
      src++;
    }
    dest->numchildren = 0;
    dest->children = 0;
  }
  else {
    unsigned int i;
    int cn;
    dest->numchildren = dtd.scaffold[src_node].childcnt;
    dest->children = *contpos;
    *contpos += dest->numchildren;
    for (i = 0, cn = dtd.scaffold[src_node].firstchild;
	 i < dest->numchildren;
	 i++, cn = dtd.scaffold[cn].nextsib) {
      build_node(parser, cn, &(dest->children[i]), contpos, strpos);
    }
    dest->name = 0;
  }
}