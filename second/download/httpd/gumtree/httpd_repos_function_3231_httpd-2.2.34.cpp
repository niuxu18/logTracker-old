static void
reportDefault(XML_Parser parser, const ENCODING *enc,
              const char *s, const char *end)
{
  if (MUST_CONVERT(enc, s)) {
    const char **eventPP;
    const char **eventEndPP;
    if (enc == encoding) {
      eventPP = &eventPtr;
      eventEndPP = &eventEndPtr;
    }
    else {
      eventPP = &(openInternalEntities->internalEventPtr);
      eventEndPP = &(openInternalEntities->internalEventEndPtr);
    }
    do {
      ICHAR *dataPtr = (ICHAR *)dataBuf;
      XmlConvert(enc, &s, end, &dataPtr, (ICHAR *)dataBufEnd);
      *eventEndPP = s;
      defaultHandler(handlerArg, dataBuf, dataPtr - (ICHAR *)dataBuf);
      *eventPP = s;
    } while (s != end);
  }
  else
    defaultHandler(handlerArg, (XML_Char *)s, (XML_Char *)end - (XML_Char *)s);
}