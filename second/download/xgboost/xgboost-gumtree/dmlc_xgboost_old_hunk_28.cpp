      #pragma omp parallel for schedule(static)
      for (bst_omp_uint i = 0; i < nsize; ++i) {
        const int tid = omp_get_thread_num();
        tree::RegTree::FVec &feats = thread_temp[tid];
        int64_t ridx = static_cast<int64_t>(batch.base_rowid + i);
        utils::Assert(static_cast<size_t>(ridx) < info.num_row, "data row index exceed bound");
        // loop over output groups
        for (int gid = 0; gid < mparam.num_output_group; ++gid) {
          this->Pred(batch[i],
