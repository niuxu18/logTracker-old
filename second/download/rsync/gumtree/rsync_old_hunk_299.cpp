  clean_flist(flist);

  if (verbose && recurse && !am_server) {
    fprintf(FINFO,"done\n");
  }

  return flist;

oom:
    out_of_memory("recv_file_list");
    return NULL; /* not reached */
}
