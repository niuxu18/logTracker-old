  int n = static_cast<int>(graph.size());
  for(int depender_index = 0; depender_index < n; ++depender_index)
    {
    NodeList const& nl = graph[depender_index];
    cmTarget* depender = this->Targets[depender_index];
    fprintf(stderr, "target %d is [%s]\n",
            depender_index, depender->GetName());
    for(NodeList::const_iterator ni = nl.begin(); ni != nl.end(); ++ni)
      {
      int dependee_index = *ni;
      cmTarget* dependee = this->Targets[dependee_index];
