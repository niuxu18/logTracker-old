{
  for (unsigned int i=0; i<10; i++)
    {
    char name[128];
    sprintf(name, "CMAKE_MATCH_%d", i);
    mf->AddDefinition(name, re.match(i).c_str());
    mf->MarkVariableAsUsed(name);
    }
}
