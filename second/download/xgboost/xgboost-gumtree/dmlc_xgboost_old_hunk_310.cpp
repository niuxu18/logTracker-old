          if (req_in[i]) min_write = std::min(links[i].size_write, min_write);
        }
        utils::Assert(min_write <= links[pid].size_read, "boundary check");
        ReturnType ret = links[pid].ReadToRingBuffer(min_write);
        if (ret != kSuccess) {
          return ReportError(&links[pid], ret);
        }
