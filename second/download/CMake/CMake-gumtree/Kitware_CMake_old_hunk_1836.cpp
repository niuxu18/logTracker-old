
  // call process directory
  inst.ProcessDirectory(passed, failed);
  total = passed + failed.size();

  if (total == 0)
    {
    std::cerr << "No tests were found!!!\n";
    }
  else
    {
    float percent = passed * 100.0 / total;
    fprintf(stderr,"%.0f%% tests passed, %i tests failed out of %i\n",
            percent, failed.size(), total);
    if (failed.size()) 
