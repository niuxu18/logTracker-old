/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmVTKWrapTclCommand.h"

// cmVTKWrapTclCommand
bool cmVTKWrapTclCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  
  // keep the library name
  this->LibraryName = argsIn[0];

  if (argsIn[1] == std::string("SOURCES"))
    {
    this->Makefile->ExpandSourceListArguments(argsIn, args, 3);
    }
  else
    {
    this->Makefile->ExpandSourceListArguments(argsIn, args, 2);
    }
  
  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  if(!this->Makefile->IsOn("VTK_WRAP_TCL"))
    {
    return true;
    }

  // extract the sources and commands parameters
  std::vector<std::string> sources;
  bool doing_sources = true;
  
  for(std::vector<std::string>::const_iterator j = (args.begin() + 1);
      j != args.end(); ++j)
    {   
    if(*j == "SOURCES")
      {
      doing_sources = true;
      }
    else if (*j == "COMMANDS")
      {
      doing_sources = false;
      }
    else
      { 
      if(doing_sources)
        {
        sources.push_back(*j);
        }
      else
        {
        this->Commands.push_back(*j);
        }
      }
    }

  // get the list of classes for this library
  if (sources.size())
    {
    // what is the current source dir
    std::string cdir = this->Makefile->GetCurrentDirectory();

    // get the resulting source list name
    this->SourceList = sources[0];
    std::string sourceListValue;
    
    // was the list already populated
    const char *def =
      this->Makefile->GetDefinition(this->SourceList.c_str());  
    if (def)
      {
      sourceListValue = def;
      sourceListValue += ";";
      }
    
    // Create the init file 
    std::string res = this->LibraryName;
    res += "Init.cxx";
    sourceListValue += res;
    
    for(std::vector<std::string>::iterator j = (sources.begin() + 1);
        j != sources.end(); ++j)
      {   
      cmSourceFile *curr = this->Makefile->GetSource(j->c_str());
      
      // if we should wrap the class
      if (!curr || !curr->GetPropertyAsBool("WRAP_EXCLUDE"))
        {
        cmSourceFile file;
        std::string srcDir = cdir;
        if (curr)
          {
          file.SetProperty("ABSTRACT",curr->GetProperty("ABSTRACT"));
          srcDir  = cmSystemTools::GetFilenamePath(curr->GetFullPath());
          }
        std::string srcName = cmSystemTools::GetFilenameWithoutExtension(*j);
        std::string newName = srcName + "Tcl";
        std::string hname = srcDir + "/" + srcName + ".h";
        file.SetName(newName.c_str(),
                     this->Makefile->GetCurrentOutputDirectory(),
                     "cxx",false);
        this->WrapHeaders.push_back(hname);
        // add starting depends
        file.GetDepends().push_back(hname);
        this->WrapClasses.push_back(file);
        sourceListValue += ";";
        sourceListValue += newName + ".cxx";
        }
      }
    // add the init file
    cmSourceFile cfile;
    cfile.SetProperty("ABSTRACT","0");
    std::string newName = this->LibraryName;
    newName += "Init";
    this->CreateInitFile(res);
    cfile.SetName(newName.c_str(),
                  this->Makefile->GetCurrentOutputDirectory(),
                  "cxx",false);
    this->Makefile->AddSource(cfile);
    this->Makefile->AddDefinition(this->SourceList.c_str(),
                                  sourceListValue.c_str());  
    }
  
  return true;
}

void cmVTKWrapTclCommand::FinalPass() 
{
  // first we add the rules for all the .h to Tcl.cxx files
  size_t lastClass = this->WrapClasses.size();
  std::vector<std::string> depends;
  const char* wtcl =
    this->Makefile->GetRequiredDefinition("VTK_WRAP_TCL_EXE");
  const char* hints = this->Makefile->GetDefinition("VTK_WRAP_HINTS");

  // wrap all the .h files
  depends.push_back(wtcl);
  if(hints)
    {
    depends.push_back(hints);
    }
  for(size_t classNum = 0; classNum < lastClass; classNum++)
    {
    this->Makefile->AddSource(this->WrapClasses[classNum]);
    cmCustomCommandLine commandLine;
    commandLine.push_back(wtcl);
    commandLine.push_back(this->WrapHeaders[classNum]);
    if(hints)
      {
      commandLine.push_back(hints);
      }
    commandLine.push_back((this->WrapClasses[classNum].
                           GetPropertyAsBool("ABSTRACT") ? "0" : "1"));
    std::string res = this->Makefile->GetCurrentOutputDirectory();
    res += "/";
    res += this->WrapClasses[classNum].GetSourceName() + ".cxx";
    commandLine.push_back(res);

    cmCustomCommandLines commandLines;
    commandLines.push_back(commandLine);
    std::vector<std::string> outputs;
    outputs.push_back(res);
    const char* no_comment = 0;
    this->Makefile->AddCustomCommandOldStyle(this->LibraryName.c_str(),
                                         outputs,
                                         depends,
                                         this->WrapHeaders[classNum].c_str(),
                                         commandLines,
                                         no_comment);
    }
}

bool cmVTKWrapTclCommand::CreateInitFile(std::string& res) 
{
  /* we have to make sure that the name is the correct case */
  std::string kitName = cmSystemTools::Capitalized(this->LibraryName);
  
  std::vector<std::string> classes;
  size_t lastClass = this->WrapHeaders.size();
  size_t classNum;
  for(classNum = 0; classNum < lastClass; classNum++)
    {
    if (!this->WrapClasses[classNum].GetPropertyAsBool("ABSTRACT"))
      {
      std::string cls = this->WrapHeaders[classNum];
      cls = cls.substr(0,cls.size()-2);
      std::string::size_type pos = cls.rfind('/');    
      if(pos != std::string::npos)
        {
        cls = cls.substr(pos+1);
        }
      classes.push_back(cls);
      }
    }
  
  // open the init file
  std::string outFileName = 
    this->Makefile->GetCurrentOutputDirectory();
  outFileName += "/" + res;
  
  return this->WriteInit(kitName.c_str(), outFileName, classes);
}


/* warning this code is also in getclasses.cxx under pcmaker */
bool cmVTKWrapTclCommand::WriteInit(const char *kitName, 
                                    std::string& outFileName,
                                    std::vector<std::string>& classes)
{
  unsigned int i;
  std::string tempOutputFile = outFileName + ".tmp";
  FILE *fout = fopen(tempOutputFile.c_str(),"w");
  if (!fout)
    {
    cmSystemTools::Error("Failed to open TclInit file for ",
                         tempOutputFile.c_str());
    cmSystemTools::ReportLastSystemError("");
    return false;
    }

  // capitalized commands just once
  std::vector<std::string> capcommands;
  for (i = 0; i < this->Commands.size(); i++)
    {
    capcommands.push_back(cmSystemTools::Capitalized(this->Commands[i]));
    }
  
  fprintf(fout,"#include \"vtkTclUtil.h\"\n");
  fprintf(fout,"#include \"vtkVersion.h\"\n");
  fprintf(fout,"#define VTK_TCL_TO_STRING(x) VTK_TCL_TO_STRING0(x)\n");
  fprintf(fout,"#define VTK_TCL_TO_STRING0(x) #x\n");
  
  fprintf(fout,
          "extern \"C\"\n"
          "{\n"
          "#if (TCL_MAJOR_VERSION == 8) && (TCL_MINOR_VERSION >= 4) "
          "&& (TCL_RELEASE_LEVEL >= TCL_FINAL_RELEASE)\n"
          "  typedef int (*vtkTclCommandType)(ClientData, Tcl_Interp *,"
          "int, CONST84 char *[]);\n"
          "#else\n"
          "  typedef int (*vtkTclCommandType)(ClientData, Tcl_Interp *,"
          "int, char *[]);\n"
          "#endif\n"
          "}\n"
          "\n");

  for (i = 0; i < classes.size(); i++)
    {
    fprintf(fout,"int %sCommand(ClientData cd, Tcl_Interp *interp,\n"
            ,classes[i].c_str());
    fprintf(fout,"             int argc, char *argv[]);\n");
    fprintf(fout,"ClientData %sNewCommand();\n",classes[i].c_str());
    }
  
  if (!strcmp(kitName,"Vtkcommontcl"))
    {
    fprintf(fout,"int vtkCommand(ClientData cd, Tcl_Interp *interp,\n"
                 "               int argc, char *argv[]);\n");
    fprintf(fout,"\nTcl_HashTable vtkInstanceLookup;\n");
    fprintf(fout,"Tcl_HashTable vtkPointerLookup;\n");
    fprintf(fout,"Tcl_HashTable vtkCommandLookup;\n");
    fprintf(fout,"int vtkCommandForward(ClientData cd, Tcl_Interp *interp,\n"
                 "                      int argc, char *argv[]){\n"
                 "  return vtkCommand(cd, interp, argc, argv);\n"
                 "}\n");
    }
  else
    {
    fprintf(fout,"\nextern Tcl_HashTable vtkInstanceLookup;\n");
    fprintf(fout,"extern Tcl_HashTable vtkPointerLookup;\n");
    fprintf(fout,"extern Tcl_HashTable vtkCommandLookup;\n");
    }
  fprintf(fout,"extern void vtkTclDeleteObjectFromHash(void *);\n");  
  fprintf(fout,"extern void vtkTclListInstances(Tcl_Interp *interp,"
          "ClientData arg);\n");

  for (i = 0; i < this->Commands.size(); i++)
    {
    fprintf(fout,
            "\nextern \"C\" {int VTK_EXPORT %s_Init(Tcl_Interp *interp);}\n",
            capcommands[i].c_str());
    }
  
  fprintf(fout,"\n\nextern \"C\" {int VTK_EXPORT "
          "%s_SafeInit(Tcl_Interp *interp);}\n", kitName);
  fprintf(fout,"\nextern \"C\" {int VTK_EXPORT %s_Init"
          "(Tcl_Interp *interp);}\n", kitName);
  
  /* create an extern ref to the generic delete function */
  fprintf(fout,"\nextern void vtkTclGenericDeleteObject(ClientData cd);\n");

  if (!strcmp(kitName,"Vtkcommontcl"))
    {
    fprintf(fout,"extern \"C\"\n{\nvoid "
            "vtkCommonDeleteAssocData(ClientData cd)\n");
    fprintf(fout,"  {\n");
    fprintf(fout,"  vtkTclInterpStruct *tis = "
            "static_cast<vtkTclInterpStruct*>(cd);\n");
    fprintf(fout,"  delete tis;\n  }\n}\n");
    }
    
  /* the main declaration */
  fprintf(fout,
          "\n\nint VTK_EXPORT %s_SafeInit(Tcl_Interp *interp)\n{\n",kitName);
  fprintf(fout,"  return %s_Init(interp);\n}\n",kitName);
  
  fprintf(fout,"\n\nint VTK_EXPORT %s_Init(Tcl_Interp *interp)\n{\n",
          kitName);
  if (!strcmp(kitName,"Vtkcommontcl"))
    {
    fprintf(fout,
            "  vtkTclInterpStruct *info = new vtkTclInterpStruct;\n");
    fprintf(fout,
            "  info->Number = 0; info->InDelete = 0; info->DebugOn = 0;\n");
    fprintf(fout,"\n");
    fprintf(fout,"\n");
    fprintf(fout,
            "  Tcl_InitHashTable(&info->InstanceLookup, TCL_STRING_KEYS);\n");
    fprintf(fout,
            "  Tcl_InitHashTable(&info->PointerLookup, TCL_STRING_KEYS);\n");
    fprintf(fout,
            "  Tcl_InitHashTable(&info->CommandLookup, TCL_STRING_KEYS);\n");
    fprintf(fout, "  Tcl_SetAssocData(interp,(char *) "
            "\"vtk\",NULL,(ClientData *)info);\n");
    fprintf(fout, "  Tcl_CreateExitHandler(vtkCommonDeleteAssocData"
            ",(ClientData *)info);\n");

    /* create special vtkCommand command */
    fprintf(fout,
            "  Tcl_CreateCommand(interp,(char *) \"vtkCommand\",\n"
            "                    reinterpret_cast<vtkTclCommandType>("
            "vtkCommandForward),\n"
            "                    (ClientData *)NULL, NULL);\n\n");
    }
  
  for (i = 0; i < this->Commands.size(); i++)
    {
    fprintf(fout,"  %s_Init(interp);\n", capcommands[i].c_str());
    }
  fprintf(fout,"\n");

  for (i = 0; i < classes.size(); i++)
    {
    fprintf(fout,"  vtkTclCreateNew(interp,(char *) \"%s\", %sNewCommand,\n",
            classes[i].c_str(), classes[i].c_str());
    fprintf(fout,"                  %sCommand);\n",classes[i].c_str());
    }
  
  fprintf(fout,"  char pkgName[]=\"%s\";\n", this->LibraryName.c_str());
  fprintf(fout,"  char pkgVers[]=VTK_TCL_TO_STRING(VTK_MAJOR_VERSION)"
               " \".\" "
               "VTK_TCL_TO_STRING(VTK_MINOR_VERSION);\n");
  fprintf(fout,"  Tcl_PkgProvide(interp, pkgName, pkgVers);\n");
  fprintf(fout,"  return TCL_OK;\n}\n");
  fclose(fout);

  // copy the file if different
  cmSystemTools::CopyFileIfDifferent(tempOutputFile.c_str(),
                                     outFileName.c_str());
  cmSystemTools::RemoveFile(tempOutputFile.c_str());

  return true;
}


