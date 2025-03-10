//----------------------------------------------------------------------------
void cmComputeLinkDepends::DisplayFinalEntries()
{
  fprintf(stderr, "target [%s] links to:\n", this->Target->GetName().c_str());
  for(std::vector<LinkEntry>::const_iterator lei =
        this->FinalLinkEntries.begin();
      lei != this->FinalLinkEntries.end(); ++lei)
    {
    if(lei->Target)
      {
      fprintf(stderr, "  target [%s]\n", lei->Target->GetName().c_str());
      }
    else
      {
