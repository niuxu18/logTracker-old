/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmCreateTestSourceList.h"


// cmCreateTestSourceList
bool cmCreateTestSourceList::InitialPass(std::vector<std::string> const& argsIn)
{
  if (argsIn.size() < 5)
    {
      this->SetError("called with wrong number of arguments.");
      return false;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);
  
  std::vector<std::string>::iterator i = args.begin();
  const char* sourceList = i->c_str();
  ++i;
  std::string driver = m_Makefile->GetCurrentOutputDirectory();
  driver += "/";
  driver += *i;
  driver += ".cxx";
  ++i;
  std::vector<std::string>::iterator testsBegin = i;
  std::ofstream fout(driver.c_str());
  if(!fout)
    {
    std::string err = "Could not create file ";
    err += driver;
    err += " for cmCreateTestSourceList command.";
    this->SetError(err.c_str());
    return false;
    }
  // Create the test driver file
  fout << "#include <stdio.h>\n";
  fout << "#include <string.h>\n";
  fout << "// forward declare test functions\n";
  for(i = testsBegin; i != args.end(); ++i)
    {
    fout << "int " << *i << "(int, char**);\n";
    }
  fout << "// Create map \n";
  fout << "typedef int (*MainFuncPointer)(int , char**);\n";
  fout << "struct functionMapEntry\n"
       << "{\n"
       << "const char* name;\n"
       << "MainFuncPointer func;\n"
       << "};\n\n";
  fout << "functionMapEntry cmakeGeneratedFunctionMapEntries[] = {\n";
  int numTests = 0;
  for(i = testsBegin; i != args.end(); ++i)
    {
    fout << "{\"" << *i << "\", " << *i << "},\n";
    numTests++;
    }
  fout << "};\n";
  fout << "int main(int ac, char** av)\n"
       << "{\n";
  fout << "  int NumTests = " << numTests << ";\n";
  fout << "  int i;\n";
  fout << "  if(ac < 2)\n";
  fout << "    {\n";
  fout << "    printf(\"Available tests:\\n\");\n";
  fout << "    for(i =0; i < NumTests; ++i)\n";
  fout << "      {\n";
  fout << "      printf(\"%d. %s\\n\", i, cmakeGeneratedFunctionMapEntries[i].name);\n";
  fout << "      }\n";
  fout << "    printf(\"To run a test, enter the test number: \");\n";
  fout << "    int testNum = 0;\n";
  fout << "    scanf(\"%d\", &testNum);\n";
  fout << "    if(testNum >= NumTests)\n";
  fout << "    {\n";
  fout << "    printf(\"%d is an invalid test number.\\n\", testNum);\n";
  fout << "    return -1;\n";
  fout << "    }\n";
  fout << "    return (*cmakeGeneratedFunctionMapEntries[testNum].func)(ac-1, av+1);\n";
  fout << "    }\n";
  fout << "  for(i =0; i < NumTests; ++i)\n";
  fout << "    {\n";
  fout << "    if(strcmp(cmakeGeneratedFunctionMapEntries[i].name, av[1]) == 0)\n";
  fout << "      {\n";
  fout << "      return (*cmakeGeneratedFunctionMapEntries[i].func)(ac-1, av+1);\n";
  fout << "      }\n";
  fout << "    }\n";
  fout << "  printf(\"Available tests:\\n\");\n";
  fout << "  for(i =0; i < NumTests; ++i)\n";
  fout << "    {\n";
  fout << "    printf(\"%d. %s\\n\", i, cmakeGeneratedFunctionMapEntries[i].name);\n";
  fout << "    }\n";
  fout << "  printf(\"Failed: %s is an invalid test name.\\n\", av[1]);\n";
  fout << "  return -1;\n";
  fout << "}\n";
  fout.close();
  // create the source list
  cmSourceFile cfile;
  cfile.SetIsAnAbstractClass(false);
  cfile.SetName(args[1].c_str(), m_Makefile->GetCurrentOutputDirectory(),
                "cxx", false);
  m_Makefile->AddSource(cfile, sourceList);
  
  for(i = testsBegin; i != args.end(); ++i)
    {
    cmSourceFile cfile;
    cfile.SetIsAnAbstractClass(false);
    cfile.SetName(i->c_str(), m_Makefile->GetCurrentDirectory(),
                  "cxx", false);
    m_Makefile->AddSource(cfile, sourceList);
    }

  return true;
}



