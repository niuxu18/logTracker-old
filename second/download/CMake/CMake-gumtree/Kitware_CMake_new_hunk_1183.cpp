       }
    } else {
      if (!isFirstLine || !startsWith(line, srcfile)) {
        if (!quiet) {
          fprintf(stdout, "%s\n", line.c_str());
        }
      } else {
