    ACLList *aclList;

    /// compiled HTTP header field value (no macros)
    Format::Format *valueFormat;

    /// internal ID for "known" headers or HDR_OTHER
    Http::HdrType fieldId;

    /// whether fieldValue may contain macros
    bool quoted;
};

/// A strtoll(10) wrapper that checks for strtoll() failures and other problems.
/// XXX: This function is not fully compatible with some HTTP syntax rules.
/// Just like strtoll(), allows whitespace prefix, a sign, and _any_ suffix.
/// Requires at least one digit to be present.
/// Sets "off" and "end" arguments if and only if no problems were found.
/// \return true if and only if no problems were found.
bool httpHeaderParseOffset(const char *start, int64_t *offPtr, char **endPtr = nullptr);

bool httpHeaderHasConnDir(const HttpHeader * hdr, const char *directive);
int httpHeaderParseInt(const char *start, int *val);
void httpHeaderPutStrf(HttpHeader * hdr, Http::HdrType id, const char *fmt,...) PRINTF_FORMAT_ARG3;

const char *getStringPrefix(const char *str, size_t len);

void httpHdrMangleList(HttpHeader *, HttpRequest *, const AccessLogEntryPointer &al, req_or_rep_t req_or_rep);

#endif

