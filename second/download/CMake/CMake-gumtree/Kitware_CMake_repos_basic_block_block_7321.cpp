f(op) {
            free(buf);
            buf = strdup(op->textoid);
          }