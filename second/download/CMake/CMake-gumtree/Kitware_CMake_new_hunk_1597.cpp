}


void
cmLocalVisualStudio6Generator
::AddUtilityCommandHack(cmTarget& target, int count,
                        std::vector<std::string>& depends,
                        const cmCustomCommandLines& commandLines)
{
  // Create a fake output that forces the rule to run.
  char* output = new char[(strlen(m_Makefile->GetStartOutputDirectory()) +
                           strlen(target.GetName()) + 30)];
  sprintf(output,"%s/%s_force_%i", m_Makefile->GetStartOutputDirectory(),
          target.GetName(), count);

  // Add the rule with the given dependencies and commands.
  const char* no_main_dependency = 0;
  const char* no_comment = 0;
  m_Makefile->AddCustomCommandToOutput(output,
                                       depends,
                                       no_main_dependency,
                                       commandLines,
                                       no_comment);

  // Replace the dependencies with the output of this rule so that the
  // next rule added will run after this one.
  depends.clear();
  depends.push_back(output);

  // Add a source file representing this output to the project.
  cmSourceFile* outsf = m_Makefile->GetSourceFileWithOutput(output);
  target.GetSourceFiles().push_back(outsf);

  // Free the fake output name.
  delete [] output;
}

void cmLocalVisualStudio6Generator::WriteCustomRule(std::ostream& fout,
                                  const char* source,
                                  const char* command,