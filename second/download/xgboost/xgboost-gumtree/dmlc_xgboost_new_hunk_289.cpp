  virtual const std::vector<size_t> &FileSize(void) const {
    return fsize_;
  }
  virtual SeekStream *Open(size_t file_index) {
    utils::Assert(file_index < fnames_.size(), "file index exceed bound"); 
    return new HDFSStream(fs_, fnames_[file_index].c_str(), "r", false);
  }
