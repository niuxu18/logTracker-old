static CURLcode wc_statemach(struct connectdata *conn)
{
  struct WildcardData * const wildcard = &(conn->data->wildcard);
  CURLcode result = CURLE_OK;

  switch (wildcard->state) {
  case CURLWC_INIT:
    result = init_wc_data(conn);
    if(wildcard->state == CURLWC_CLEAN)
      /* only listing! */
      break;
    else
      wildcard->state = result ? CURLWC_ERROR : CURLWC_MATCHING;
    break;

  case CURLWC_MATCHING: {
    /* In this state is LIST response successfully parsed, so lets restore
       previous WRITEFUNCTION callback and WRITEDATA pointer */
    struct ftp_wc_tmpdata *ftp_tmp = wildcard->tmp;
    conn->data->set.fwrite_func = ftp_tmp->backup.write_function;
    conn->data->set.out = ftp_tmp->backup.file_descriptor;
    ftp_tmp->backup.write_function = ZERO_NULL;
    ftp_tmp->backup.file_descriptor = NULL;
    wildcard->state = CURLWC_DOWNLOADING;

    if(Curl_ftp_parselist_geterror(ftp_tmp->parser)) {
      /* error found in LIST parsing */
      wildcard->state = CURLWC_CLEAN;
      return wc_statemach(conn);
    }
    else if(wildcard->filelist->size == 0) {
      /* no corresponding file */
      wildcard->state = CURLWC_CLEAN;
      return CURLE_REMOTE_FILE_NOT_FOUND;
    }
    return wc_statemach(conn);
  }

  case CURLWC_DOWNLOADING: {
    /* filelist has at least one file, lets get first one */
    struct ftp_conn *ftpc = &conn->proto.ftpc;
    struct curl_fileinfo *finfo = wildcard->filelist->head->ptr;

    char *tmp_path = aprintf("%s%s", wildcard->path, finfo->filename);
    if(!tmp_path)
      return CURLE_OUT_OF_MEMORY;

    /* switch default "state.pathbuffer" and tmp_path, good to see
       ftp_parse_url_path function to understand this trick */
    Curl_safefree(conn->data->state.pathbuffer);
    conn->data->state.pathbuffer = tmp_path;
    conn->data->state.path = tmp_path;

    infof(conn->data, "Wildcard - START of \"%s\"\n", finfo->filename);
    if(conn->data->set.chunk_bgn) {
      long userresponse = conn->data->set.chunk_bgn(
          finfo, wildcard->customptr, (int)wildcard->filelist->size);
      switch(userresponse) {
      case CURL_CHUNK_BGN_FUNC_SKIP:
        infof(conn->data, "Wildcard - \"%s\" skipped by user\n",
              finfo->filename);
        wildcard->state = CURLWC_SKIP;
        return wc_statemach(conn);
      case CURL_CHUNK_BGN_FUNC_FAIL:
        return CURLE_CHUNK_FAILED;
      }
    }

    if(finfo->filetype != CURLFILETYPE_FILE) {
      wildcard->state = CURLWC_SKIP;
      return wc_statemach(conn);
    }

    if(finfo->flags & CURLFINFOFLAG_KNOWN_SIZE)
      ftpc->known_filesize = finfo->size;

    result = ftp_parse_url_path(conn);
    if(result)
      return result;

    /* we don't need the Curl_fileinfo of first file anymore */
    Curl_llist_remove(wildcard->filelist, wildcard->filelist->head, NULL);

    if(wildcard->filelist->size == 0) { /* remains only one file to down. */
      wildcard->state = CURLWC_CLEAN;
      /* after that will be ftp_do called once again and no transfer
         will be done because of CURLWC_CLEAN state */
      return CURLE_OK;
    }
  } break;

  case CURLWC_SKIP: {
    if(conn->data->set.chunk_end)
      conn->data->set.chunk_end(conn->data->wildcard.customptr);
    Curl_llist_remove(wildcard->filelist, wildcard->filelist->head, NULL);
    wildcard->state = (wildcard->filelist->size == 0) ?
                      CURLWC_CLEAN : CURLWC_DOWNLOADING;
    return wc_statemach(conn);
  }

  case CURLWC_CLEAN: {
    struct ftp_wc_tmpdata *ftp_tmp = wildcard->tmp;
    result = CURLE_OK;
    if(ftp_tmp)
      result = Curl_ftp_parselist_geterror(ftp_tmp->parser);

    wildcard->state = result ? CURLWC_ERROR : CURLWC_DONE;
  } break;

  case CURLWC_DONE:
  case CURLWC_ERROR:
    break;
  }

  return result;
}