{
      pos = cacheSize.find(" KB");
      if (pos != std::string::npos) {
        cacheSize = cacheSize.substr(0, pos);
      }
      this->Features.L1CacheSize += atoi(cacheSize.c_str());
    }