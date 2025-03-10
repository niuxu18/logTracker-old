{
    case XML_TOK_TRAILING_CR:
      if (haveMore) {
        *nextPtr = s;
        return XML_ERROR_NONE;
      }
      *eventEndPP = end;
      if (characterDataHandler) {
        XML_Char c = 0xA;
        characterDataHandler(handlerArg, &c, 1);
      }
      else if (defaultHandler)
        reportDefault(parser, enc, s, end);
      /* We are at the end of the final buffer, should we check for
         XML_SUSPENDED, XML_FINISHED?
      */
      if (startTagLevel == 0)
        return XML_ERROR_NO_ELEMENTS;
      if (tagLevel != startTagLevel)
        return XML_ERROR_ASYNC_ENTITY;
      *nextPtr = end;
      return XML_ERROR_NONE;
    case XML_TOK_NONE:
      if (haveMore) {
        *nextPtr = s;
        return XML_ERROR_NONE;
      }
      if (startTagLevel > 0) {
        if (tagLevel != startTagLevel)
          return XML_ERROR_ASYNC_ENTITY;
        *nextPtr = s;
        return XML_ERROR_NONE;
      }
      return XML_ERROR_NO_ELEMENTS;
    case XML_TOK_INVALID:
      *eventPP = next;
      return XML_ERROR_INVALID_TOKEN;
    case XML_TOK_PARTIAL:
      if (haveMore) {
        *nextPtr = s;
        return XML_ERROR_NONE;
      }
      return XML_ERROR_UNCLOSED_TOKEN;
    case XML_TOK_PARTIAL_CHAR:
      if (haveMore) {
        *nextPtr = s;
        return XML_ERROR_NONE;
      }
      return XML_ERROR_PARTIAL_CHAR;
    case XML_TOK_ENTITY_REF:
      {
        const XML_Char *name;
        ENTITY *entity;
        XML_Char ch = (XML_Char) XmlPredefinedEntityName(enc,
                                              s + enc->minBytesPerChar,
                                              next - enc->minBytesPerChar);
        if (ch) {
          if (characterDataHandler)
            characterDataHandler(handlerArg, &ch, 1);
          else if (defaultHandler)
            reportDefault(parser, enc, s, next);
          break;
        }
        name = poolStoreString(&dtd->pool, enc,
                                s + enc->minBytesPerChar,
                                next - enc->minBytesPerChar);
        if (!name)
          return XML_ERROR_NO_MEMORY;
        entity = (ENTITY *)lookup(parser, &dtd->generalEntities, name, 0);
        poolDiscard(&dtd->pool);
        /* First, determine if a check for an existing declaration is needed;
           if yes, check that the entity exists, and that it is internal,
           otherwise call the skipped entity or default handler.
        */
        if (!dtd->hasParamEntityRefs || dtd->standalone) {
          if (!entity)
            return XML_ERROR_UNDEFINED_ENTITY;
          else if (!entity->is_internal)
            return XML_ERROR_ENTITY_DECLARED_IN_PE;
        }
        else if (!entity) {
          if (skippedEntityHandler)
            skippedEntityHandler(handlerArg, name, 0);
          else if (defaultHandler)
            reportDefault(parser, enc, s, next);
          break;
        }
        if (entity->open)
          return XML_ERROR_RECURSIVE_ENTITY_REF;
        if (entity->notation)
          return XML_ERROR_BINARY_ENTITY_REF;
        if (entity->textPtr) {
          enum XML_Error result;
          if (!defaultExpandInternalEntities) {
            if (skippedEntityHandler)
              skippedEntityHandler(handlerArg, entity->name, 0);
            else if (defaultHandler)
              reportDefault(parser, enc, s, next);
            break;
          }
          result = processInternalEntity(parser, entity, XML_FALSE);
          if (result != XML_ERROR_NONE)
            return result;
        }
        else if (externalEntityRefHandler) {
          const XML_Char *context;
          entity->open = XML_TRUE;
          context = getContext(parser);
          entity->open = XML_FALSE;
          if (!context)
            return XML_ERROR_NO_MEMORY;
          if (!externalEntityRefHandler(externalEntityRefHandlerArg,
                                        context,
                                        entity->base,
                                        entity->systemId,
                                        entity->publicId))
            return XML_ERROR_EXTERNAL_ENTITY_HANDLING;
          poolDiscard(&tempPool);
        }
        else if (defaultHandler)
          reportDefault(parser, enc, s, next);
        break;
      }
    case XML_TOK_START_TAG_NO_ATTS:
      /* fall through */
    case XML_TOK_START_TAG_WITH_ATTS:
      {
        TAG *tag;
        enum XML_Error result;
        XML_Char *toPtr;
        if (freeTagList) {
          tag = freeTagList;
          freeTagList = freeTagList->parent;
        }
        else {
          tag = (TAG *)MALLOC(sizeof(TAG));
          if (!tag)
            return XML_ERROR_NO_MEMORY;
          tag->buf = (char *)MALLOC(INIT_TAG_BUF_SIZE);
          if (!tag->buf) {
            FREE(tag);
            return XML_ERROR_NO_MEMORY;
          }
          tag->bufEnd = tag->buf + INIT_TAG_BUF_SIZE;
        }
        tag->bindings = NULL;
        tag->parent = tagStack;
        tagStack = tag;
        tag->name.localPart = NULL;
        tag->name.prefix = NULL;
        tag->rawName = s + enc->minBytesPerChar;
        tag->rawNameLength = XmlNameLength(enc, tag->rawName);
        ++tagLevel;
        {
          const char *rawNameEnd = tag->rawName + tag->rawNameLength;
          const char *fromPtr = tag->rawName;
          toPtr = (XML_Char *)tag->buf;
          for (;;) {
            int bufSize;
            int convLen;
            const enum XML_Convert_Result convert_res = XmlConvert(enc,
                       &fromPtr, rawNameEnd,
                       (ICHAR **)&toPtr, (ICHAR *)tag->bufEnd - 1);
            convLen = (int)(toPtr - (XML_Char *)tag->buf);
            if ((fromPtr >= rawNameEnd) || (convert_res == XML_CONVERT_INPUT_INCOMPLETE)) {
              tag->name.strLen = convLen;
              break;
            }
            bufSize = (int)(tag->bufEnd - tag->buf) << 1;
            {
              char *temp = (char *)REALLOC(tag->buf, bufSize);
              if (temp == NULL)
                return XML_ERROR_NO_MEMORY;
              tag->buf = temp;
              tag->bufEnd = temp + bufSize;
              toPtr = (XML_Char *)temp + convLen;
            }
          }
        }
        tag->name.str = (XML_Char *)tag->buf;
        *toPtr = XML_T('\0');
        result = storeAtts(parser, enc, s, &(tag->name), &(tag->bindings));
        if (result)
          return result;
        if (startElementHandler)
          startElementHandler(handlerArg, tag->name.str,
                              (const XML_Char **)atts);
        else if (defaultHandler)
          reportDefault(parser, enc, s, next);
        poolClear(&tempPool);
        break;
      }
    case XML_TOK_EMPTY_ELEMENT_NO_ATTS:
      /* fall through */
    case XML_TOK_EMPTY_ELEMENT_WITH_ATTS:
      {
        const char *rawName = s + enc->minBytesPerChar;
        enum XML_Error result;
        BINDING *bindings = NULL;
        XML_Bool noElmHandlers = XML_TRUE;
        TAG_NAME name;
        name.str = poolStoreString(&tempPool, enc, rawName,
                                   rawName + XmlNameLength(enc, rawName));
        if (!name.str)
          return XML_ERROR_NO_MEMORY;
        poolFinish(&tempPool);
        result = storeAtts(parser, enc, s, &name, &bindings);
        if (result != XML_ERROR_NONE) {
          freeBindings(parser, bindings);
          return result;
        }
        poolFinish(&tempPool);
        if (startElementHandler) {
          startElementHandler(handlerArg, name.str, (const XML_Char **)atts);
          noElmHandlers = XML_FALSE;
        }
        if (endElementHandler) {
          if (startElementHandler)
            *eventPP = *eventEndPP;
          endElementHandler(handlerArg, name.str);
          noElmHandlers = XML_FALSE;
        }
        if (noElmHandlers && defaultHandler)
          reportDefault(parser, enc, s, next);
        poolClear(&tempPool);
        freeBindings(parser, bindings);
      }
      if (tagLevel == 0)
        return epilogProcessor(parser, next, end, nextPtr);
      break;
    case XML_TOK_END_TAG:
      if (tagLevel == startTagLevel)
        return XML_ERROR_ASYNC_ENTITY;
      else {
        int len;
        const char *rawName;
        TAG *tag = tagStack;
        tagStack = tag->parent;
        tag->parent = freeTagList;
        freeTagList = tag;
        rawName = s + enc->minBytesPerChar*2;
        len = XmlNameLength(enc, rawName);
        if (len != tag->rawNameLength
            || memcmp(tag->rawName, rawName, len) != 0) {
          *eventPP = rawName;
          return XML_ERROR_TAG_MISMATCH;
        }
        --tagLevel;
        if (endElementHandler) {
          const XML_Char *localPart;
          const XML_Char *prefix;
          XML_Char *uri;
          localPart = tag->name.localPart;
          if (ns && localPart) {
            /* localPart and prefix may have been overwritten in
               tag->name.str, since this points to the binding->uri
               buffer which gets re-used; so we have to add them again
            */
            uri = (XML_Char *)tag->name.str + tag->name.uriLen;
            /* don't need to check for space - already done in storeAtts() */
            while (*localPart) *uri++ = *localPart++;
            prefix = (XML_Char *)tag->name.prefix;
            if (ns_triplets && prefix) {
              *uri++ = namespaceSeparator;
              while (*prefix) *uri++ = *prefix++;
             }
            *uri = XML_T('\0');
          }
          endElementHandler(handlerArg, tag->name.str);
        }
        else if (defaultHandler)
          reportDefault(parser, enc, s, next);
        while (tag->bindings) {
          BINDING *b = tag->bindings;
          if (endNamespaceDeclHandler)
            endNamespaceDeclHandler(handlerArg, b->prefix->name);
          tag->bindings = tag->bindings->nextTagBinding;
          b->nextTagBinding = freeBindingList;
          freeBindingList = b;
          b->prefix->binding = b->prevPrefixBinding;
        }
        if (tagLevel == 0)
          return epilogProcessor(parser, next, end, nextPtr);
      }
      break;
    case XML_TOK_CHAR_REF:
      {
        int n = XmlCharRefNumber(enc, s);
        if (n < 0)
          return XML_ERROR_BAD_CHAR_REF;
        if (characterDataHandler) {
          XML_Char buf[XML_ENCODE_MAX];
          characterDataHandler(handlerArg, buf, XmlEncode(n, (ICHAR *)buf));
        }
        else if (defaultHandler)
          reportDefault(parser, enc, s, next);
      }
      break;
    case XML_TOK_XML_DECL:
      return XML_ERROR_MISPLACED_XML_PI;
    case XML_TOK_DATA_NEWLINE:
      if (characterDataHandler) {
        XML_Char c = 0xA;
        characterDataHandler(handlerArg, &c, 1);
      }
      else if (defaultHandler)
        reportDefault(parser, enc, s, next);
      break;
    case XML_TOK_CDATA_SECT_OPEN:
      {
        enum XML_Error result;
        if (startCdataSectionHandler)
          startCdataSectionHandler(handlerArg);
#if 0
        /* Suppose you doing a transformation on a document that involves
           changing only the character data.  You set up a defaultHandler
           and a characterDataHandler.  The defaultHandler simply copies
           characters through.  The characterDataHandler does the
           transformation and writes the characters out escaping them as
           necessary.  This case will fail to work if we leave out the
           following two lines (because & and < inside CDATA sections will
           be incorrectly escaped).

           However, now we have a start/endCdataSectionHandler, so it seems
           easier to let the user deal with this.
        */
        else if (characterDataHandler)
          characterDataHandler(handlerArg, dataBuf, 0);
#endif
        else if (defaultHandler)
          reportDefault(parser, enc, s, next);
        result = doCdataSection(parser, enc, &next, end, nextPtr, haveMore);
        if (result != XML_ERROR_NONE)
          return result;
        else if (!next) {
          processor = cdataSectionProcessor;
          return result;
        }
      }
      break;
    case XML_TOK_TRAILING_RSQB:
      if (haveMore) {
        *nextPtr = s;
        return XML_ERROR_NONE;
      }
      if (characterDataHandler) {
        if (MUST_CONVERT(enc, s)) {
          ICHAR *dataPtr = (ICHAR *)dataBuf;
          XmlConvert(enc, &s, end, &dataPtr, (ICHAR *)dataBufEnd);
          characterDataHandler(handlerArg, dataBuf,
                               (int)(dataPtr - (ICHAR *)dataBuf));
        }
        else
          characterDataHandler(handlerArg,
                               (XML_Char *)s,
                               (int)((XML_Char *)end - (XML_Char *)s));
      }
      else if (defaultHandler)
        reportDefault(parser, enc, s, end);
      /* We are at the end of the final buffer, should we check for
         XML_SUSPENDED, XML_FINISHED?
      */
      if (startTagLevel == 0) {
        *eventPP = end;
        return XML_ERROR_NO_ELEMENTS;
      }
      if (tagLevel != startTagLevel) {
        *eventPP = end;
        return XML_ERROR_ASYNC_ENTITY;
      }
      *nextPtr = end;
      return XML_ERROR_NONE;
    case XML_TOK_DATA_CHARS:
      {
        XML_CharacterDataHandler charDataHandler = characterDataHandler;
        if (charDataHandler) {
          if (MUST_CONVERT(enc, s)) {
            for (;;) {
              ICHAR *dataPtr = (ICHAR *)dataBuf;
              const enum XML_Convert_Result convert_res = XmlConvert(enc, &s, next, &dataPtr, (ICHAR *)dataBufEnd);
              *eventEndPP = s;
              charDataHandler(handlerArg, dataBuf,
                              (int)(dataPtr - (ICHAR *)dataBuf));
              if ((convert_res == XML_CONVERT_COMPLETED) || (convert_res == XML_CONVERT_INPUT_INCOMPLETE))
                break;
              *eventPP = s;
            }
          }
          else
            charDataHandler(handlerArg,
                            (XML_Char *)s,
                            (int)((XML_Char *)next - (XML_Char *)s));
        }
        else if (defaultHandler)
          reportDefault(parser, enc, s, next);
      }
      break;
    case XML_TOK_PI:
      if (!reportProcessingInstruction(parser, enc, s, next))
        return XML_ERROR_NO_MEMORY;
      break;
    case XML_TOK_COMMENT:
      if (!reportComment(parser, enc, s, next))
        return XML_ERROR_NO_MEMORY;
      break;
    default:
      if (defaultHandler)
        reportDefault(parser, enc, s, next);
      break;
    }