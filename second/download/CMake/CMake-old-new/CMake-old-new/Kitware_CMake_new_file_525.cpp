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
#include "cmLocalUnixMakefileGenerator3.h"

#include "cmDepends.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmMakefileTargetGenerator.h"
#include "cmSourceFile.h"
#include "cmake.h"
#include "cmVersion.h"

// Include dependency scanners for supported languages.  Only the
// C/C++ scanner is needed for bootstrapping CMake.
#include "cmDependsC.h"
#ifdef CMAKE_BUILD_WITH_CMAKE
# include "cmDependsFortran.h"
# include "cmDependsJava.h"
# include <cmsys/Terminal.h>
#endif

#include <memory> // auto_ptr
#include <queue>

//----------------------------------------------------------------------------
cmLocalUnixMakefileGenerator3::cmLocalUnixMakefileGenerator3()
{
  this->SilentNoColon = false;
  this->WindowsShell = false;
  this->IncludeDirective = "include";
  this->MakefileVariableSize = 0;
  this->IgnoreLibPrefix = false;
  this->PassMakeflags = false;
  this->DefineWindowsNULL = false;
  this->UnixCD = true;
  this->ColorMakefile = false;
  this->SkipPreprocessedSourceRules = false;
  this->SkipAssemblySourceRules = false;
  this->NativeEchoCommand = "@echo ";
  this->NativeEchoWindows = true;
  this->MakeCommandEscapeTargetTwice = false;
  this->IsMakefileGenerator = true;
  this->BorlandMakeCurlyHack = false;
}

//----------------------------------------------------------------------------
cmLocalUnixMakefileGenerator3::~cmLocalUnixMakefileGenerator3()
{
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::Configure()
{
  // Compute the path to use when referencing the current output
  // directory from the top output directory.
  this->HomeRelativeOutputPath =
    this->Convert(this->Makefile->GetStartOutputDirectory(), HOME_OUTPUT);
  if(this->HomeRelativeOutputPath == ".")
    {
    this->HomeRelativeOutputPath = "";
    }
  if(!this->HomeRelativeOutputPath.empty())
    {
    this->HomeRelativeOutputPath += "/";
    }
  this->cmLocalGenerator::Configure();
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::Generate()
{
  // Store the configuration name that will be generated.
  if(const char* config = this->Makefile->GetDefinition("CMAKE_BUILD_TYPE"))
    {
    // Use the build type given by the user.
    this->ConfigurationName = config;
    }
  else
    {
    // No configuration type given.
    this->ConfigurationName = "";
    }

  // Record whether some options are enabled to avoid checking many
  // times later.
  this->ColorMakefile = this->Makefile->IsOn("CMAKE_COLOR_MAKEFILE");
  this->SkipPreprocessedSourceRules =
    this->Makefile->IsOn("CMAKE_SKIP_PREPROCESSED_SOURCE_RULES");
  this->SkipAssemblySourceRules =
    this->Makefile->IsOn("CMAKE_SKIP_ASSEMBLY_SOURCE_RULES");

  // Generate the rule files for each target.
  cmTargets& targets = this->Makefile->GetTargets();
  std::string empty;
  for(cmTargets::iterator t = targets.begin(); t != targets.end(); ++t)
    {
    cmMakefileTargetGenerator *tg = 
      cmMakefileTargetGenerator::New(this, t->first, &(t->second));
    if (tg)
      {
      this->TargetGenerators.push_back(tg);
      tg->WriteRuleFiles();
      }
    }

  // write the local Makefile
  this->WriteLocalMakefile();
  
  // Write the cmake file with information for this directory.
  this->WriteDirectoryInformationFile();
}

//----------------------------------------------------------------------------
// return info about progress actions
unsigned long cmLocalUnixMakefileGenerator3::GetNumberOfProgressActions()
{
  unsigned long result = 0;

  for (std::vector<cmMakefileTargetGenerator *>::iterator mtgIter = 
         this->TargetGenerators.begin();
       mtgIter != this->TargetGenerators.end(); ++mtgIter)
    {
    result += (*mtgIter)->GetNumberOfProgressActions();
    }  
  return result;
}

//----------------------------------------------------------------------------
// return info about progress actions
unsigned long cmLocalUnixMakefileGenerator3
::GetNumberOfProgressActionsForTarget(const char *name)
{
  for (std::vector<cmMakefileTargetGenerator *>::iterator mtgIter = 
         this->TargetGenerators.begin();
       mtgIter != this->TargetGenerators.end(); ++mtgIter)
    {
    if (!strcmp(name,(*mtgIter)->GetTargetName()))
      {
      return (*mtgIter)->GetNumberOfProgressActions();
      }
    }  
  return 0;
}


//----------------------------------------------------------------------------
// writes the progreess variables and also closes out the targets
void cmLocalUnixMakefileGenerator3
::WriteProgressVariables(unsigned long total,
                         unsigned long &current)
{
  // delete the makefile target generator objects
  for (std::vector<cmMakefileTargetGenerator *>::iterator mtgIter = 
         this->TargetGenerators.begin();
       mtgIter != this->TargetGenerators.end(); ++mtgIter)
    {
    (*mtgIter)->WriteProgressVariables(total,current);
    delete *mtgIter;
    }  
  this->TargetGenerators.clear();
}

void cmLocalUnixMakefileGenerator3::WriteAllProgressVariable()
{
  // write the top level progress for the all target
  std::string progressFile = cmake::GetCMakeFilesDirectory();
  progressFile += "/progress.make";
  std::string progressFileNameFull = 
    this->ConvertToFullPath(progressFile.c_str());
  cmGeneratedFileStream ruleFileStream(progressFileNameFull.c_str());
  if(!ruleFileStream)
    {
    return;
    }

  cmGlobalUnixMakefileGenerator3 *gg = 
    static_cast<cmGlobalUnixMakefileGenerator3*>(this->GlobalGenerator);

  ruleFileStream << "CMAKE_ALL_PROGRESS = " 
                 << gg->GetNumberOfProgressActionsInAll(this) << "\n";
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::WriteLocalMakefile()
{
  // generate the includes
  std::string ruleFileName = "Makefile";

  // Open the rule file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  std::string ruleFileNameFull = this->ConvertToFullPath(ruleFileName);
  cmGeneratedFileStream ruleFileStream(ruleFileNameFull.c_str());
  if(!ruleFileStream)
    {
    return;
    }
  // always write the top makefile
  if (this->Parent)
    {
    ruleFileStream.SetCopyIfDifferent(true);
    }
  
  // write the all rules
  this->WriteLocalAllRules(ruleFileStream);
  
  // only write local targets unless at the top Keep track of targets already
  // listed.
  std::set<cmStdString> emittedTargets;
  if (this->Parent)
    {
    // write our targets, and while doing it collect up the object
    // file rules
    this->WriteLocalMakefileTargets(ruleFileStream,emittedTargets);
    }
  else
    {
    cmGlobalUnixMakefileGenerator3 *gg = 
      static_cast<cmGlobalUnixMakefileGenerator3*>(this->GlobalGenerator);
    gg->WriteConvenienceRules(ruleFileStream,emittedTargets);
    }

  bool do_preprocess_rules =
    this->GetCreatePreprocessedSourceRules();
  bool do_assembly_rules =
    this->GetCreateAssemblySourceRules();

  // now write out the object rules
  // for each object file name
  for (std::map<cmStdString, LocalObjectInfo>::iterator lo = 
         this->LocalObjectFiles.begin();
       lo != this->LocalObjectFiles.end(); ++lo)
    {
    // Add a convenience rule for building the object file.
    this->WriteObjectConvenienceRule(ruleFileStream,
                                     "target to build an object file",
                                     lo->first.c_str(), lo->second);

    // Check whether preprocessing and assembly rules make sense.
    // They make sense only for C and C++ sources.
    bool lang_is_c_or_cxx = false;
    for(std::vector<LocalObjectEntry>::const_iterator ei =
          lo->second.begin(); ei != lo->second.end(); ++ei)
      {
      if(ei->Language == "C" || ei->Language == "CXX")
        {
        lang_is_c_or_cxx = true;
        }
      }

    // Add convenience rules for preprocessed and assembly files.
    if(lang_is_c_or_cxx && (do_preprocess_rules || do_assembly_rules))
      {
      std::string::size_type dot_pos = lo->first.rfind(".");
      std::string base = lo->first.substr(0, dot_pos);
      if(do_preprocess_rules)
        {
        this->WriteObjectConvenienceRule(
          ruleFileStream, "target to preprocess a source file",
          (base + ".i").c_str(), lo->second);
        }
      if(do_assembly_rules)
        {
        this->WriteObjectConvenienceRule(
          ruleFileStream, "target to generate assembly for a file",
          (base + ".s").c_str(), lo->second);
        }
      }
    }

  // add a help target as long as there isn;t a real target named help
  if(emittedTargets.insert("help").second)
    {
    cmGlobalUnixMakefileGenerator3 *gg = 
      static_cast<cmGlobalUnixMakefileGenerator3*>(this->GlobalGenerator);
    gg->WriteHelpRule(ruleFileStream,this);
    }

  this->WriteSpecialTargetsBottom(ruleFileStream);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteObjectConvenienceRule(std::ostream& ruleFileStream,
                             const char* comment, const char* output,
                             LocalObjectInfo const& targets)
{
  // Recursively make the rule for each target using the object file.
  std::vector<std::string> commands;
  for(std::vector<LocalObjectEntry>::const_iterator t = targets.begin();
      t != targets.end(); ++t)
    {
    std::string tgtMakefileName =
      this->GetRelativeTargetDirectory(*(t->Target));
    std::string targetName = tgtMakefileName;
    tgtMakefileName += "/build.make";
    targetName += "/";
    targetName += output;
    commands.push_back(
      this->GetRecursiveMakeCall(tgtMakefileName.c_str(), targetName.c_str())
      );
    this->CreateCDCommand(commands,
                          this->Makefile->GetHomeOutputDirectory(),
                          this->Makefile->GetStartOutputDirectory());
    }

  // Write the rule to the makefile.
  std::vector<std::string> no_depends;
  this->WriteMakeRule(ruleFileStream, comment,
                      output, no_depends, commands, true, true);
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3
::WriteLocalMakefileTargets(std::ostream& ruleFileStream,
                            std::set<cmStdString> &emitted)
{
  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // for each target we just provide a rule to cd up to the top and do a make
  // on the target
  cmTargets& targets = this->Makefile->GetTargets();
  std::string localName;
  for(cmTargets::iterator t = targets.begin(); t != targets.end(); ++t)
    {
    if((t->second.GetType() == cmTarget::EXECUTABLE) ||
       (t->second.GetType() == cmTarget::STATIC_LIBRARY) ||
       (t->second.GetType() == cmTarget::SHARED_LIBRARY) ||
       (t->second.GetType() == cmTarget::MODULE_LIBRARY) ||
       (t->second.GetType() == cmTarget::UTILITY))
      {
      emitted.insert(t->second.GetName());

      // for subdirs add a rule to build this specific target by name.
      localName = this->GetRelativeTargetDirectory(t->second);
      localName += "/rule";
      commands.clear();
      depends.clear();
      
      // Build the target for this pass.
      std::string makefile2 = cmake::GetCMakeFilesDirectoryPostSlash();
      makefile2 += "Makefile2";
      commands.push_back(this->GetRecursiveMakeCall
                         (makefile2.c_str(),localName.c_str()));
      this->CreateCDCommand(commands,
                            this->Makefile->GetHomeOutputDirectory(),
                            this->Makefile->GetStartOutputDirectory());
      this->WriteMakeRule(ruleFileStream, "Convenience name for target.",
                          localName.c_str(), depends, commands, true);
      
      // Add a target with the canonical name (no prefix, suffix or path).
      if(localName != t->second.GetName())
        {
        commands.clear();
        depends.push_back(localName);
        this->WriteMakeRule(ruleFileStream, "Convenience name for target.",
                            t->second.GetName(), depends, commands, true);
        }

      // Add a fast rule to build the target
      std::string makefileName = this->GetRelativeTargetDirectory(t->second);
      makefileName += "/build.make";
      // make sure the makefile name is suitable for a makefile
      std::string makeTargetName = 
        this->GetRelativeTargetDirectory(t->second);
      makeTargetName += "/build";
      localName = t->second.GetName();
      localName += "/fast";
      depends.clear();
      commands.clear();
      commands.push_back(this->GetRecursiveMakeCall
                         (makefileName.c_str(), makeTargetName.c_str()));
      this->CreateCDCommand(commands,
                            this->Makefile->GetHomeOutputDirectory(),
                            this->Makefile->GetStartOutputDirectory());
      this->WriteMakeRule(ruleFileStream, "fast build rule for target.",
                          localName.c_str(), depends, commands, true);

      // Add a local name for the rule to relink the target before
      // installation.
      if(t->second.NeedRelinkBeforeInstall())
        {
        makeTargetName = this->GetRelativeTargetDirectory(t->second);
        makeTargetName += "/preinstall";
        localName = t->second.GetName();
        localName += "/preinstall";
        depends.clear();
        commands.clear();
        commands.push_back(this->GetRecursiveMakeCall
                           (makefile2.c_str(), makeTargetName.c_str()));
        this->CreateCDCommand(commands,
                              this->Makefile->GetHomeOutputDirectory(),
                              this->Makefile->GetStartOutputDirectory());
        this->WriteMakeRule(ruleFileStream,
                            "Manual pre-install relink rule for target.",
                            localName.c_str(), depends, commands, true);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::WriteDirectoryInformationFile()
{
  std::string infoFileName = this->Makefile->GetStartOutputDirectory();
  infoFileName += cmake::GetCMakeFilesDirectory();
  infoFileName += "/CMakeDirectoryInformation.cmake";

  // Open the output file.
  cmGeneratedFileStream infoFileStream(infoFileName.c_str());
  if(!infoFileStream)
    {
    return;
    }

  // Write the do not edit header.
  this->WriteDisclaimer(infoFileStream);

  // Setup relative path conversion tops.
  infoFileStream
    << "# Relative path conversion top directories.\n"
    << "SET(CMAKE_RELATIVE_PATH_TOP_SOURCE \"" << this->RelativePathTopSource
    << "\")\n"
    << "SET(CMAKE_RELATIVE_PATH_TOP_BINARY \"" << this->RelativePathTopBinary
    << "\")\n"
    << "\n";

  // Tell the dependency scanner to use unix paths if necessary.
  if(cmSystemTools::GetForceUnixPaths())
    {
    infoFileStream
      << "# Force unix paths in dependencies.\n"
      << "SET(CMAKE_FORCE_UNIX_PATHS 1)\n"
      << "\n";
    }

  // Store the include search path for this directory.
  infoFileStream
    << "# The C and CXX include file search paths:\n";
  infoFileStream
    << "SET(CMAKE_C_INCLUDE_PATH\n";
  std::vector<std::string> includeDirs;
  this->GetIncludeDirectories(includeDirs, false);
  for(std::vector<std::string>::iterator i = includeDirs.begin();
      i != includeDirs.end(); ++i)
    {
    infoFileStream
      << "  \"" << this->Convert(i->c_str(),HOME_OUTPUT).c_str() << "\"\n";
    }
  infoFileStream
    << "  )\n";
  infoFileStream
    << "SET(CMAKE_CXX_INCLUDE_PATH ${CMAKE_C_INCLUDE_PATH})\n";
  infoFileStream
    << "SET(CMAKE_Fortran_INCLUDE_PATH ${CMAKE_C_INCLUDE_PATH})\n";

  // Store the include regular expressions for this directory.
  infoFileStream
    << "\n"
    << "# The C and CXX include file regular expressions for "
    << "this directory.\n";
  infoFileStream
    << "SET(CMAKE_C_INCLUDE_REGEX_SCAN ";
  this->WriteCMakeArgument(infoFileStream,
                           this->Makefile->GetIncludeRegularExpression());
  infoFileStream
    << ")\n";
  infoFileStream
    << "SET(CMAKE_C_INCLUDE_REGEX_COMPLAIN ";
  this->WriteCMakeArgument(infoFileStream,
                           this->Makefile->GetComplainRegularExpression());
  infoFileStream
    << ")\n";
  infoFileStream
    << "SET(CMAKE_CXX_INCLUDE_REGEX_SCAN ${CMAKE_C_INCLUDE_REGEX_SCAN})\n";
  infoFileStream
    << "SET(CMAKE_CXX_INCLUDE_REGEX_COMPLAIN "
    "${CMAKE_C_INCLUDE_REGEX_COMPLAIN})\n";
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::ConvertToFullPath(const std::string& localPath)
{
  std::string dir = this->Makefile->GetStartOutputDirectory();
  dir += "/";
  dir += localPath;
  return dir;
}


const std::string &cmLocalUnixMakefileGenerator3::GetHomeRelativeOutputPath()
{
  return this->HomeRelativeOutputPath;
}


//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteMakeRule(std::ostream& os,
                const char* comment,
                const char* target,
                const std::vector<std::string>& depends,
                const std::vector<std::string>& commands,
                bool symbolic,
                bool in_help)
{
  // Make sure there is a target.
  if(!target || !*target)
    {
    cmSystemTools::Error("No target for WriteMakeRule! called with comment: ",
                         comment);
    return;
    }

  std::string replace;

  // Write the comment describing the rule in the makefile.
  if(comment)
    {
    replace = comment;
    std::string::size_type lpos = 0;
    std::string::size_type rpos;
    while((rpos = replace.find('\n', lpos)) != std::string::npos)
      {
      os << "# " << replace.substr(lpos, rpos-lpos) << "\n";
      lpos = rpos+1;
      }
    os << "# " << replace.substr(lpos) << "\n";
    }

  // Construct the left hand side of the rule.
  replace = target;
  std::string tgt = this->Convert(replace.c_str(),HOME_OUTPUT,MAKEFILE);
  const char* space = "";
  if(tgt.size() == 1)
    {
    // Add a space before the ":" to avoid drive letter confusion on
    // Windows.
    space = " ";
    }

  // Mark the rule as symbolic if requested.
  if(symbolic)
    {
    if(const char* sym =
       this->Makefile->GetDefinition("CMAKE_MAKE_SYMBOLIC_RULE"))
      {
      os << tgt.c_str() << space << ": " << sym << "\n";
      }
    }

  // Write the rule.
  if(depends.empty())
    {
    // No dependencies.  The commands will always run.
    os << tgt.c_str() << space << ":\n";
    }
  else
    {
    // Split dependencies into multiple rule lines.  This allows for
    // very long dependency lists even on older make implementations.
    for(std::vector<std::string>::const_iterator dep = depends.begin();
        dep != depends.end(); ++dep)
      {
      replace = *dep;
      replace = this->Convert(replace.c_str(),HOME_OUTPUT,MAKEFILE);
      os << tgt.c_str() << space << ": " << replace.c_str() << "\n";
      }
    }

  // Write the list of commands.
  for(std::vector<std::string>::const_iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    replace = *i;
    os << "\t" << replace.c_str() << "\n";
    }
  os << "\n";

  // Add the output to the local help if requested.
  if(in_help)
    {
    this->LocalHelp.push_back(target);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteMakeVariables(std::ostream& makefileStream)
{
  this->WriteDivider(makefileStream);
  makefileStream
    << "# Set environment variables for the build.\n"
    << "\n";
  if(this->DefineWindowsNULL)
    {
    makefileStream
      << "!IF \"$(OS)\" == \"Windows_NT\"\n"
      << "NULL=\n"
      << "!ELSE\n"
      << "NULL=nul\n"
      << "!ENDIF\n";
    }
  if(this->WindowsShell)
    {
     makefileStream
       << "SHELL = cmd.exe\n"
       << "\n";
    }
  else
    {
      makefileStream
        << "# The shell in which to execute make rules.\n"
        << "SHELL = /bin/sh\n"
        << "\n";
    }

  std::string cmakecommand =
      this->Makefile->GetRequiredDefinition("CMAKE_COMMAND");
  makefileStream
    << "# The CMake executable.\n"
    << "CMAKE_COMMAND = "
    << this->Convert(cmakecommand.c_str(), FULL, SHELL).c_str() 
    << "\n"
    << "\n";
  makefileStream
    << "# The command to remove a file.\n"
    << "RM = "
    << this->Convert(cmakecommand.c_str(),FULL,SHELL).c_str()
    << " -E remove -f\n"
    << "\n";
  
  if(this->Makefile->GetDefinition("CMAKE_EDIT_COMMAND"))
    {
    makefileStream
      << "# The program to use to edit the cache.\n"
      << "CMAKE_EDIT_COMMAND = "
      << (this->ConvertToOutputForExisting(
            this->Makefile->GetDefinition("CMAKE_EDIT_COMMAND"))) << "\n"
      << "\n";
    }

  makefileStream
    << "# The top-level source directory on which CMake was run.\n"
    << "CMAKE_SOURCE_DIR = "
    << this->Convert(this->Makefile->GetHomeDirectory(), FULL, SHELL)
    << "\n"
    << "\n";
  makefileStream
    << "# The top-level build directory on which CMake was run.\n"
    << "CMAKE_BINARY_DIR = "
    << this->Convert(this->Makefile->GetHomeOutputDirectory(), FULL, SHELL)
    << "\n"
    << "\n";
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteSpecialTargetsTop(std::ostream& makefileStream)
{
  this->WriteDivider(makefileStream);
  makefileStream
    << "# Special targets provided by cmake.\n"
    << "\n";

  std::vector<std::string> no_commands;
  std::vector<std::string> no_depends;

  // Special target to cleanup operation of make tool.
  // This should be the first target except for the default_target in
  // the interface Makefile.
  this->WriteMakeRule(
    makefileStream, "Disable implicit rules so canoncical targets will work.",
    ".SUFFIXES", no_depends, no_commands, false);

  // Add a fake suffix to keep HP happy.  Must be max 32 chars for SGI make.
  std::vector<std::string> depends;
  depends.push_back(".hpux_make_needs_suffix_list");
  this->WriteMakeRule(makefileStream, 0,
                      ".SUFFIXES", depends, no_commands, false);

  cmGlobalUnixMakefileGenerator3* gg =
    static_cast<cmGlobalUnixMakefileGenerator3*>(this->GlobalGenerator);
  // Write special target to silence make output.  This must be after
  // the default target in case VERBOSE is set (which changes the
  // name).  The setting of CMAKE_VERBOSE_MAKEFILE to ON will cause a
  // "VERBOSE=1" to be added as a make variable which will change the
  // name of this special target.  This gives a make-time choice to
  // the user.
  if((this->Makefile->IsOn("CMAKE_VERBOSE_MAKEFILE")) 
     || (gg->GetForceVerboseMakefiles()))
    {
    makefileStream
      << "# Produce verbose output by default.\n"
      << "VERBOSE = 1\n"
      << "\n";
    }
  if(this->SilentNoColon)
    {
    makefileStream << "$(VERBOSE).SILENT\n";
    }
  else
    {
    this->WriteMakeRule(makefileStream,
                        "Suppress display of executed commands.",
                        "$(VERBOSE).SILENT",
                        no_depends,
                        no_commands, false);
    }

  // Work-around for makes that drop rules that have no dependencies
  // or commands.
  std::string hack = gg->GetEmptyRuleHackDepends();
  if(!hack.empty())
    {
    no_depends.push_back(hack);
    }
  std::string hack_cmd = gg->GetEmptyRuleHackCommand();
  if(!hack_cmd.empty())
    {
    no_commands.push_back(hack_cmd);
    }

  // Special symbolic target that never exists to force dependers to
  // run their rules.
  this->WriteMakeRule
    (makefileStream,
     "A target that is always out of date.",
     "cmake_force", no_depends, no_commands, true);

  // Variables for reference by other rules.
  this->WriteMakeVariables(makefileStream);
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3
::WriteSpecialTargetsBottom(std::ostream& makefileStream)
{
  this->WriteDivider(makefileStream);
  makefileStream
    << "# Special targets to cleanup operation of make.\n"
    << "\n";

  // Write special "cmake_check_build_system" target to run cmake with
  // the --check-build-system flag.
  {
  // Build command to run CMake to check if anything needs regenerating.
  std::string cmakefileName = cmake::GetCMakeFilesDirectoryPostSlash();
  cmakefileName += "Makefile.cmake";
  std::string runRule =
    "$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)";
  runRule += " --check-build-system ";
  runRule += this->Convert(cmakefileName.c_str(),NONE,SHELL);
  runRule += " 0";

  std::vector<std::string> no_depends;
  std::vector<std::string> commands;
  commands.push_back(runRule);
  if(this->Parent)
    {
    this->CreateCDCommand(commands,
                          this->Makefile->GetHomeOutputDirectory(),
                          this->Makefile->GetStartOutputDirectory());
    }
  this->WriteMakeRule(makefileStream,
                      "Special rule to run CMake to check the build system "
                      "integrity.\n"
                      "No rule that depends on this can have "
                      "commands that come from listfiles\n"
                      "because they might be regenerated.",
                      "cmake_check_build_system",
                      no_depends,
                      commands, true);
  }
}



//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteConvenienceRule(std::ostream& ruleFileStream,
                       const char* realTarget,
                       const char* helpTarget)
{
  // A rule is only needed if the names are different.
  if(strcmp(realTarget, helpTarget) != 0)
    {
    // The helper target depends on the real target.
    std::vector<std::string> depends;
    depends.push_back(realTarget);

    // There are no commands.
    std::vector<std::string> no_commands;

    // Write the rule.
    this->WriteMakeRule(ruleFileStream, "Convenience name for target.",
                        helpTarget, depends, no_commands, true);
    }
}


//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3::GetRelativeTargetDirectory(cmTarget& target)
{
  std::string dir = this->HomeRelativeOutputPath;
  dir += this->GetTargetDirectory(target);
  return this->Convert(dir.c_str(),NONE,UNCHANGED);
}



//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::AppendFlags(std::string& flags,
                                                const char* newFlags)
{
  if(this->WatcomWMake && newFlags && *newFlags)
    {
    std::string newf = newFlags;
    if(newf.find("\\\"") != newf.npos)
      {
      cmSystemTools::ReplaceString(newf, "\\\"", "\"");
      this->cmLocalGenerator::AppendFlags(flags, newf.c_str());
      return;
      }
    }
  this->cmLocalGenerator::AppendFlags(flags, newFlags);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendRuleDepend(std::vector<std::string>& depends,
                   const char* ruleFileName)
{
  // Add a dependency on the rule file itself unless an option to skip
  // it is specifically enabled by the user or project.
  const char* nodep = 
    this->Makefile->GetDefinition("CMAKE_SKIP_RULE_DEPENDENCY");
  if(!nodep || cmSystemTools::IsOff(nodep))
    {
    depends.push_back(ruleFileName);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCustomDepends(std::vector<std::string>& depends,
                      const std::vector<cmCustomCommand>& ccs)
{
  for(std::vector<cmCustomCommand>::const_iterator i = ccs.begin();
      i != ccs.end(); ++i)
    {
    this->AppendCustomDepend(depends, *i);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCustomDepend(std::vector<std::string>& depends,
                     const cmCustomCommand& cc)
{
  for(std::vector<std::string>::const_iterator d = cc.GetDepends().begin();
      d != cc.GetDepends().end(); ++d)
    {
    // Lookup the real name of the dependency in case it is a CMake target.
    std::string dep = this->GetRealDependency
      (d->c_str(), this->ConfigurationName.c_str());
    depends.push_back(dep);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCustomCommands(std::vector<std::string>& commands,
                       const std::vector<cmCustomCommand>& ccs)
{
  for(std::vector<cmCustomCommand>::const_iterator i = ccs.begin();
      i != ccs.end(); ++i)
    {
    this->AppendCustomCommand(commands, *i, true);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCustomCommand(std::vector<std::string>& commands,
                      const cmCustomCommand& cc, bool echo_comment)
{
  // Optionally create a command to display the custom command's
  // comment text.  This is used for pre-build, pre-link, and
  // post-build command comments.  Custom build step commands have
  // their comments generated elsewhere.
  if(echo_comment)
    {
    const char* comment = cc.GetComment();
    if(comment && *comment)
      {
      this->AppendEcho(commands, comment,
                       cmLocalUnixMakefileGenerator3::EchoGenerate);
      }
    }

  // if the command specified a working directory use it.
  const char* dir  = this->Makefile->GetStartOutputDirectory();
  const char* workingDir = cc.GetWorkingDirectory();
  if(workingDir)
    {
    dir = workingDir;
    }
  bool escapeOldStyle = cc.GetEscapeOldStyle();
  bool escapeAllowMakeVars = cc.GetEscapeAllowMakeVars();

  // Add each command line to the set of commands.
  std::vector<std::string> commands1;
  for(cmCustomCommandLines::const_iterator cl = cc.GetCommandLines().begin();
      cl != cc.GetCommandLines().end(); ++cl)
    {
    // Build the command line in a single string.
    const cmCustomCommandLine& commandLine = *cl;
    std::string cmd = GetRealLocation(commandLine[0].c_str(), 
                                      this->ConfigurationName.c_str());
    if (cmd.size())
      {
      cmSystemTools::ReplaceString(cmd, "/./", "/");
      // Convert the command to a relative path only if the current
      // working directory will be the start-output directory.
      bool had_slash = cmd.find("/") != cmd.npos;
      if(!workingDir)
        {
        cmd = this->Convert(cmd.c_str(),START_OUTPUT);
        }
      bool has_slash = cmd.find("/") != cmd.npos;
      if(had_slash && !has_slash)
        {
        // This command was specified as a path to a file in the
        // current directory.  Add a leading "./" so it can run
        // without the current directory being in the search path.
        cmd = "./" + cmd;
        }
      cmd = this->Convert(cmd.c_str(),NONE,SHELL);
      for(unsigned int j=1; j < commandLine.size(); ++j)
        {
        cmd += " ";
        if(escapeOldStyle)
          {
          cmd += this->EscapeForShellOldStyle(commandLine[j].c_str());
          }
        else
          {
          cmd += this->EscapeForShell(commandLine[j].c_str(),
                                      escapeAllowMakeVars);
          }
        }
      if(this->BorlandMakeCurlyHack)
        {
        // Borland Make has a very strange bug.  If the first curly
        // brace anywhere in the command string is a left curly, it
        // must be written {{} instead of just {.  Otherwise some
        // curly braces are removed.  The hack can be skipped if the
        // first curly brace is the last character.
        std::string::size_type lcurly = cmd.find("{");
        if(lcurly != cmd.npos && lcurly < (cmd.size()-1))
          {
          std::string::size_type rcurly = cmd.find("}");
          if(rcurly == cmd.npos || rcurly > lcurly)
            {
            // The first curly is a left curly.  Use the hack.
            std::string hack_cmd = cmd.substr(0, lcurly);
            hack_cmd += "{{}";
            hack_cmd += cmd.substr(lcurly+1);
            cmd = hack_cmd;
            }
          }
        }
      commands1.push_back(cmd);
      }
    }

  // Setup the proper working directory for the commands.
  this->CreateCDCommand(commands1, dir,
                        this->Makefile->GetHomeOutputDirectory());

  // push back the custom commands
  commands.insert(commands.end(), commands1.begin(), commands1.end());
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCleanCommand(std::vector<std::string>& commands,
                     const std::vector<std::string>& files,
                     cmTarget& target, const char* filename)
{
  if(!files.empty())
    {
    std::string cleanfile = this->Makefile->GetCurrentOutputDirectory();
    cleanfile += "/";
    cleanfile += this->GetTargetDirectory(target);
    cleanfile += "/cmake_clean";
    if(filename)
      {
      cleanfile += "_";
      cleanfile += filename;
      }
    cleanfile += ".cmake";
    std::string cleanfilePath = this->Convert(cleanfile.c_str(), FULL);
    std::ofstream fout(cleanfilePath.c_str());
    if(!fout)
      {
      cmSystemTools::Error("Could not create ", cleanfilePath.c_str());
      }
    fout << "FILE(REMOVE\n";
    std::string remove = "$(CMAKE_COMMAND) -P ";
    remove += this->Convert(cleanfile.c_str(), START_OUTPUT, SHELL);
    for(std::vector<std::string>::const_iterator f = files.begin();
        f != files.end(); ++f)
      {
      fout << "\"" << this->Convert(f->c_str(),START_OUTPUT,UNCHANGED) 
           << "\"\n";
      }
    fout << ")\n";
    commands.push_back(remove);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3::AppendEcho(std::vector<std::string>& commands,
                                          const char* text,
                                          EchoColor color)
{
  // Choose the color for the text.
  std::string color_name;
#ifdef CMAKE_BUILD_WITH_CMAKE
  if(this->GlobalGenerator->GetToolSupportsColor() && this->ColorMakefile)
    {
    // See cmake::ExecuteEchoColor in cmake.cxx for these options.
    // This color set is readable on both black and white backgrounds.
    switch(color)
      {
      case EchoNormal:
        break;
      case EchoDepend:
        color_name = "--magenta --bold ";
        break;
      case EchoBuild:
        color_name = "--green ";
        break;
      case EchoLink:
        color_name = "--red --bold ";
        break;
      case EchoGenerate:
        color_name = "--blue --bold ";
        break;
      case EchoGlobal:
        color_name = "--cyan ";
        break;
      }
    }
#else
  (void)color;
#endif

  // Echo one line at a time.
  std::string line;
  line.reserve(200);
  for(const char* c = text;; ++c)
    {
    if(*c == '\n' || *c == '\0')
      {
      // Avoid writing a blank last line on end-of-string.
      if(*c != '\0' || !line.empty())
        {
        // Add a command to echo this line.
        std::string cmd;
        if(color_name.empty())
          {
          // Use the native echo command.
          cmd = this->NativeEchoCommand;
          cmd += this->EscapeForShell(line.c_str(), false,
                                      this->NativeEchoWindows);
          }
        else
          {
          // Use cmake to echo the text in color.
          cmd = "@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) ";
          cmd += color_name;
          cmd += this->EscapeForShell(line.c_str());
          }
        commands.push_back(cmd);
        }

      // Reset the line to emtpy.
      line = "";

      // Terminate on end-of-string.
      if(*c == '\0')
        {
        return;
        }
      }
    else if(*c != '\r')
      {
      // Append this character to the current line.
      line += *c;
      }
    }
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::CreateMakeVariable(const char* sin, const char* s2in)
{
  std::string s = sin;
  std::string s2 = s2in;
  std::string unmodified = s;
  unmodified += s2;
  // if there is no restriction on the length of make variables
  // and there are no "." charactors in the string, then return the
  // unmodified combination.
  if((!this->MakefileVariableSize && unmodified.find('.') == s.npos)
     && (!this->MakefileVariableSize && unmodified.find('-') == s.npos))
    {
    return unmodified;
    }

  // see if the variable has been defined before and return
  // the modified version of the variable
  std::map<cmStdString, cmStdString>::iterator i = 
    this->MakeVariableMap.find(unmodified);
  if(i != this->MakeVariableMap.end())
    {
    return i->second;
    }
  // start with the unmodified variable
  std::string ret = unmodified;
  // if this there is no value for this->MakefileVariableSize then
  // the string must have bad characters in it
  if(!this->MakefileVariableSize)
    {
    cmSystemTools::ReplaceString(ret, ".", "_");
    cmSystemTools::ReplaceString(ret, "-", "__");
    int ni = 0;
    char buffer[5];
    // make sure the _ version is not already used, if
    // it is used then add number to the end of the variable
    while(this->ShortMakeVariableMap.count(ret) && ni < 1000)
      {
      ++ni;
      sprintf(buffer, "%04d", ni);
      ret = unmodified + buffer;
      }
    this->ShortMakeVariableMap[ret] = "1";
    this->MakeVariableMap[unmodified] = ret;
    return ret;
    }

  // if the string is greater the 32 chars it is an invalid vairable name
  // for borland make
  if(static_cast<int>(ret.size()) > this->MakefileVariableSize)
    {
    int keep = this->MakefileVariableSize - 8;
    int size = keep + 3;
    std::string str1 = s;
    std::string str2 = s2;
    // we must shorten the combined string by 4 charactors
    // keep no more than 24 charactors from the second string
    if(static_cast<int>(str2.size()) > keep)
      {
      str2 = str2.substr(0, keep);
      }
    if(static_cast<int>(str1.size()) + static_cast<int>(str2.size()) > size)
      {
      str1 = str1.substr(0, size - str2.size());
      }
    char buffer[5];
    int ni = 0;
    sprintf(buffer, "%04d", ni);
    ret = str1 + str2 + buffer;
    while(this->ShortMakeVariableMap.count(ret) && ni < 1000)
      {
      ++ni;
      sprintf(buffer, "%04d", ni);
      ret = str1 + str2 + buffer;
      }
    if(ni == 1000)
      {
      cmSystemTools::Error("Borland makefile variable length too long");
      return unmodified;
      }
    // once an unused variable is found
    this->ShortMakeVariableMap[ret] = "1";
    }
  // always make an entry into the unmodified to variable map
  this->MakeVariableMap[unmodified] = ret;
  return ret;
}

//----------------------------------------------------------------------------
bool cmLocalUnixMakefileGenerator3::UpdateDependencies(const char* tgtInfo,
                                                       bool verbose,
                                                       bool color)
{
  // read in the target info file
  if(!this->Makefile->ReadListFile(0, tgtInfo) ||
     cmSystemTools::GetErrorOccuredFlag())
    {
    cmSystemTools::Error("Target DependInfo.cmake file not found");
    }

  // Check if any multiple output pairs have a missing file.
  this->CheckMultipleOutputs(verbose);

  std::string dir = cmSystemTools::GetFilenamePath(tgtInfo);
  std::string internalDependFile = dir + "/depend.internal";
  std::string dependFile = dir + "/depend.make";

  // Check the implicit dependencies to see if they are up to date.
  // The build.make file may have explicit dependencies for the object
  // files but these will not affect the scanning process so they need
  // not be considered.
  cmDependsC checker;
  checker.SetVerbose(verbose);
  checker.SetFileComparison
    (this->GlobalGenerator->GetCMakeInstance()->GetFileComparison());
  if(!checker.Check(dependFile.c_str(), internalDependFile.c_str()))
    {
    // The dependencies must be regenerated.
    std::string targetName = cmSystemTools::GetFilenameName(dir);
    targetName = targetName.substr(0, targetName.length()-4);
    std::string message = "Scanning dependencies of target ";
    message += targetName;
#ifdef CMAKE_BUILD_WITH_CMAKE
    cmSystemTools::MakefileColorEcho(
      cmsysTerminal_Color_ForegroundMagenta |
      cmsysTerminal_Color_ForegroundBold,
      message.c_str(), true, color);
#else
    fprintf(stdout, "%s\n", message.c_str());
#endif

    return this->ScanDependencies(dir.c_str());
    }
  else
    {
    // The dependencies are already up-to-date.
    return true;
    }
}

//----------------------------------------------------------------------------
bool
cmLocalUnixMakefileGenerator3
::ScanDependencies(const char* targetDir)
{
  // Read the directory information file.
  cmMakefile* mf = this->Makefile;
  bool haveDirectoryInfo = false;
  std::string dirInfoFile = this->Makefile->GetStartOutputDirectory();
  dirInfoFile += cmake::GetCMakeFilesDirectory();
  dirInfoFile += "/CMakeDirectoryInformation.cmake";
  if(mf->ReadListFile(0, dirInfoFile.c_str()) &&
     !cmSystemTools::GetErrorOccuredFlag())
    {
    haveDirectoryInfo = true;
    }
  
  // Lookup useful directory information.
  if(haveDirectoryInfo)
    {
    // Test whether we need to force Unix paths.
    if(const char* force = mf->GetDefinition("CMAKE_FORCE_UNIX_PATHS"))
      {
      if(!cmSystemTools::IsOff(force))
        {
        cmSystemTools::SetForceUnixPaths(true);
        }
      }

    // Setup relative path top directories.
    this->RelativePathsConfigured = true;
    if(const char* relativePathTopSource =
       mf->GetDefinition("CMAKE_RELATIVE_PATH_TOP_SOURCE"))
      {
      this->RelativePathTopSource = relativePathTopSource;
      }
    if(const char* relativePathTopBinary =
       mf->GetDefinition("CMAKE_RELATIVE_PATH_TOP_BINARY"))
      {
      this->RelativePathTopBinary = relativePathTopBinary;
      }
    }
  else
    {
    cmSystemTools::Error("Directory Information file not found");
    }

  // create the file stream for the depends file
  std::string dir = targetDir;
  
  // Open the rule file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  std::string ruleFileNameFull = dir;
  ruleFileNameFull += "/depend.make";
  cmGeneratedFileStream ruleFileStream(ruleFileNameFull.c_str());
  ruleFileStream.SetCopyIfDifferent(true);
  if(!ruleFileStream)
    {
    return false;
    }
  std::string internalRuleFileNameFull = dir;
  internalRuleFileNameFull += "/depend.internal";
  cmGeneratedFileStream 
    internalRuleFileStream(internalRuleFileNameFull.c_str());
  internalRuleFileStream.SetCopyIfDifferent(true);
  if(!internalRuleFileStream)
    {
    return false;
    }

  this->WriteDisclaimer(ruleFileStream);
  this->WriteDisclaimer(internalRuleFileStream);

  // for each language we need to scan, scan it 
  const char *langStr = mf->GetSafeDefinition("CMAKE_DEPENDS_LANGUAGES");
  std::vector<std::string> langs;
  cmSystemTools::ExpandListArgument(langStr, langs);
  for (std::vector<std::string>::iterator li = 
         langs.begin(); li != langs.end(); ++li)
    {
    // construct the checker
    std::string lang = li->c_str();
    
    // Get the set of include directories.
    std::vector<std::string> includes;
    if(haveDirectoryInfo)
      {
      std::string includePathVar = "CMAKE_";
      includePathVar += lang;
      includePathVar += "_INCLUDE_PATH";
      if(const char* includePath = mf->GetDefinition(includePathVar.c_str()))
        {
        cmSystemTools::ExpandListArgument(includePath, includes);
        }
      }
    
    // Get the include file regular expression.
    std::string includeRegexScan = "^.*$";
    std::string includeRegexComplain = "^$";
    if(haveDirectoryInfo)
      {
      std::string scanRegexVar = "CMAKE_";
      scanRegexVar += lang;
      scanRegexVar += "_INCLUDE_REGEX_SCAN";
      if(const char* scanRegex = mf->GetDefinition(scanRegexVar.c_str()))
        {
        includeRegexScan = scanRegex;
        }
      std::string complainRegexVar = "CMAKE_";
      complainRegexVar += lang;
      complainRegexVar += "_INCLUDE_REGEX_COMPLAIN";
      if(const char* complainRegex = 
         mf->GetDefinition(complainRegexVar.c_str()))
        {
        includeRegexComplain = complainRegex;
        }
      }

    // Create the scanner for this language
    cmDepends *scanner = 0;
    if(lang == "C" || lang == "CXX" || lang == "RC")
      {
      std::string includeCacheFileName = dir;
      includeCacheFileName += "/";
      includeCacheFileName += lang;
      includeCacheFileName += ".includecache";
      
      // TODO: Handle RC (resource files) dependencies correctly.
      scanner = new cmDependsC(includes,
                               includeRegexScan.c_str(),
                               includeRegexComplain.c_str(),
                               includeCacheFileName);
      }
#ifdef CMAKE_BUILD_WITH_CMAKE
    else if(lang == "Fortran")
      {
      scanner = new cmDependsFortran(includes, dir);
      }
    else if(lang == "Java")
      {
      scanner = new cmDependsJava();
      }
#endif
    
    if (scanner)
      {
      scanner->SetLocalGenerator(this);
      scanner->SetFileComparison
        (this->GlobalGenerator->GetCMakeInstance()->GetFileComparison());
      // for each file we need to scan
      std::string srcLang = "CMAKE_DEPENDS_CHECK_";
      srcLang += lang;
      const char *srcStr = mf->GetSafeDefinition(srcLang.c_str());
      std::vector<std::string> srcs;
      cmSystemTools::ExpandListArgument(srcStr, srcs);
      for (std::vector<std::string>::iterator si = 
        srcs.begin(); si != srcs.end(); ++si)
        {
        std::string &src = *si;
        ++si;
        // make sure the object file is relative to home output
        std::string obj = *si;
        obj = this->Convert(obj.c_str(),HOME_OUTPUT,MAKEFILE);
        scanner->Write(src.c_str(),obj.c_str(),
                       ruleFileStream, internalRuleFileStream);
        }

      // free the scanner for this language
      delete scanner;
      }
    }

  return true;
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::CheckMultipleOutputs(bool verbose)
{
  cmMakefile* mf = this->Makefile;

  // Get the string listing the multiple output pairs.
  const char* pairs_string = mf->GetDefinition("CMAKE_MULTIPLE_OUTPUT_PAIRS");
  if(!pairs_string)
    {
    return;
    }

  // Convert the string to a list and preserve empty entries.
  std::vector<std::string> pairs;
  cmSystemTools::ExpandListArgument(pairs_string, pairs, true);
  for(std::vector<std::string>::const_iterator i = pairs.begin();
      i != pairs.end(); ++i)
    {
    const std::string& depender = *i;
    if(++i != pairs.end())
      {
      const std::string& dependee = *i;

      // If the depender is missing then delete the dependee to make
      // sure both will be regenerated.
      if(cmSystemTools::FileExists(dependee.c_str()) &&
         !cmSystemTools::FileExists(depender.c_str()))
        {
        if(verbose)
          {
          cmOStringStream msg;
          msg << "Deleting primary custom command output \"" << dependee
              << "\" because another output \""
              << depender << "\" does not exist." << std::endl;
          cmSystemTools::Stdout(msg.str().c_str());
          }
        cmSystemTools::RemoveFile(dependee.c_str());
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3
::WriteLocalAllRules(std::ostream& ruleFileStream)
{
  this->WriteDisclaimer(ruleFileStream);

  // Write the main entry point target.  This must be the VERY first
  // target so that make with no arguments will run it.
  {
  // Just depend on the all target to drive the build.
  std::vector<std::string> depends;
  std::vector<std::string> no_commands;
  depends.push_back("all");

  // Write the rule.
  this->WriteMakeRule(ruleFileStream,
                      "Default target executed when no arguments are "
                      "given to make.",
                      "default_target",
                      depends,
                      no_commands, true);
  }

  this->WriteSpecialTargetsTop(ruleFileStream);

  // Include the progress variables for the target.
  std::string progressFile = cmake::GetCMakeFilesDirectory();
  progressFile += "/progress.make";
  std::string progressFileNameFull =
    this->ConvertToFullPath(progressFile.c_str());
  ruleFileStream
    << "# Include the progress variables for this target.\n"
    << this->IncludeDirective << " "
    << this->Convert(progressFileNameFull.c_str(),
                     cmLocalGenerator::START_OUTPUT,
                     cmLocalGenerator::MAKEFILE) << "\n\n";
  
  // Write all global targets
  this->WriteDivider(ruleFileStream);
  ruleFileStream
    << "# Targets provided globally by CMake.\n"
    << "\n";
  cmTargets* targets = &(this->Makefile->GetTargets());
  cmTargets::iterator glIt;
  for ( glIt = targets->begin(); glIt != targets->end(); ++ glIt )
    {
    if ( glIt->second.GetType() == cmTarget::GLOBAL_TARGET )
      {
      std::string targetString = "Special rule for the target " + glIt->first;
      std::vector<std::string> commands;
      std::vector<std::string> depends;

      const char* text = glIt->second.GetProperty("EchoString");
      if ( !text )
        {
        text = "Running external command ...";
        }
      std::set<cmStdString>::const_iterator dit;
      for ( dit = glIt->second.GetUtilities().begin();
         dit != glIt->second.GetUtilities().end();
        ++ dit )
        {
        depends.push_back(dit->c_str());
        }
      this->AppendEcho(commands, text,
                       cmLocalUnixMakefileGenerator3::EchoGlobal);

      // Global targets store their rules in pre- and post-build commands.
      this->AppendCustomDepends(depends,   
                                glIt->second.GetPreBuildCommands());
      this->AppendCustomDepends(depends,   
                                glIt->second.GetPostBuildCommands());
      this->AppendCustomCommands(commands, 
                                 glIt->second.GetPreBuildCommands());
      this->AppendCustomCommands(commands, 
                                 glIt->second.GetPostBuildCommands());
      std::string targetName = glIt->second.GetName();
      this->WriteMakeRule(ruleFileStream, targetString.c_str(), 
                          targetName.c_str(), depends, commands, true);

      // Provide a "/fast" version of the target.
      depends.clear();
      if((targetName == "install") 
          || (targetName == "install_local")
          || (targetName == "install_strip"))
        {
        // Provide a fast install target that does not depend on all
        // but has the same command.
        depends.push_back("preinstall/fast");
        }
      else
        {
        // Just forward to the real target so at least it will work.
        depends.push_back(targetName);
        commands.clear();
        }
      targetName += "/fast";
      this->WriteMakeRule(ruleFileStream, targetString.c_str(),
                          targetName.c_str(), depends, commands, true);
      }
    }

  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // Write the all rule.
  std::string dir;
  std::string recursiveTarget = this->Makefile->GetStartOutputDirectory();
  recursiveTarget += "/all";

  depends.push_back("cmake_check_build_system");

  std::string progressDir = this->Makefile->GetHomeOutputDirectory();
  progressDir += cmake::GetCMakeFilesDirectory();
    {
    cmOStringStream progCmd;
    progCmd << 
      "$(CMAKE_COMMAND) -E cmake_progress_start ";
    progCmd << this->Convert(progressDir.c_str(),
                             cmLocalGenerator::FULL,
                             cmLocalGenerator::SHELL);
    progCmd << " $(CMAKE_ALL_PROGRESS)";
    commands.push_back(progCmd.str());
    }
  std::string mf2Dir = cmake::GetCMakeFilesDirectoryPostSlash();
  mf2Dir += "Makefile2";
  commands.push_back(this->GetRecursiveMakeCall(mf2Dir.c_str(),
                                                recursiveTarget.c_str()));
  this->CreateCDCommand(commands,
                        this->Makefile->GetHomeOutputDirectory(),
                        this->Makefile->GetStartOutputDirectory());
    {
    cmOStringStream progCmd;
    progCmd << "$(CMAKE_COMMAND) -E cmake_progress_start "; // # 0
    progCmd << this->Convert(progressDir.c_str(),
                             cmLocalGenerator::FULL,
                             cmLocalGenerator::SHELL);
    progCmd << " 0";
    commands.push_back(progCmd.str());
    }
  this->WriteMakeRule(ruleFileStream, "The main all target", "all",
                      depends, commands, true);

  // Write the clean rule.
  recursiveTarget = this->Makefile->GetStartOutputDirectory();
  recursiveTarget += "/clean";
  commands.clear();
  depends.clear();
  commands.push_back(this->GetRecursiveMakeCall(mf2Dir.c_str(),
                                                recursiveTarget.c_str()));
  this->CreateCDCommand(commands,
                                this->Makefile->GetHomeOutputDirectory(),
                                this->Makefile->GetStartOutputDirectory());
  this->WriteMakeRule(ruleFileStream, "The main clean target", "clean",
                      depends, commands, true);
  commands.clear();
  depends.clear();
  depends.push_back("clean");
  this->WriteMakeRule(ruleFileStream, "The main clean target", "clean/fast",
                      depends, commands, true);

  // Write the preinstall rule.
  recursiveTarget = this->Makefile->GetStartOutputDirectory();
  recursiveTarget += "/preinstall";
  commands.clear();
  depends.clear();
  const char* noall =
    this->Makefile->GetDefinition("CMAKE_SKIP_INSTALL_ALL_DEPENDENCY");
  if(!noall || cmSystemTools::IsOff(noall))
    {
    // Drive the build before installing.
    depends.push_back("all");
    }
  else
    {
    // At least make sure the build system is up to date.
    depends.push_back("cmake_check_build_system");
    }
  commands.push_back
    (this->GetRecursiveMakeCall(mf2Dir.c_str(), recursiveTarget.c_str()));
  this->CreateCDCommand(commands,
                        this->Makefile->GetHomeOutputDirectory(),
                        this->Makefile->GetStartOutputDirectory());
  this->WriteMakeRule(ruleFileStream, "Prepare targets for installation.",
                      "preinstall", depends, commands, true);
  depends.clear();
  this->WriteMakeRule(ruleFileStream, "Prepare targets for installation.",
                      "preinstall/fast", depends, commands, true);

  // write the depend rule, really a recompute depends rule
  depends.clear();
  commands.clear();
  std::string cmakefileName = cmake::GetCMakeFilesDirectoryPostSlash();
  cmakefileName += "Makefile.cmake";
  std::string runRule =
    "$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)";
  runRule += " --check-build-system ";
  runRule += this->Convert(cmakefileName.c_str(),cmLocalGenerator::NONE,
                           cmLocalGenerator::SHELL);
  runRule += " 1";
  commands.push_back(runRule);
  this->CreateCDCommand(commands,
                        this->Makefile->GetHomeOutputDirectory(),
                        this->Makefile->GetStartOutputDirectory());
  this->WriteMakeRule(ruleFileStream, "clear depends", 
                      "depend", 
                      depends, commands, true);
}


//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::ClearDependencies(cmMakefile* mf,
                                                      bool verbose)
{
  // Get the list of target files to check
  const char* infoDef = mf->GetDefinition("CMAKE_DEPEND_INFO_FILES");
  if(!infoDef)
    {
    return;
    }
  std::vector<std::string> files;
  cmSystemTools::ExpandListArgument(infoDef, files);

  // Each depend information file corresponds to a target.  Clear the
  // dependencies for that target.
  cmDepends clearer;
  clearer.SetVerbose(verbose);
  for(std::vector<std::string>::iterator l = files.begin();
      l != files.end(); ++l)
    {
    std::string dir = cmSystemTools::GetFilenamePath(l->c_str());

    // Clear the implicit dependency makefile.
    std::string dependFile = dir + "/depend.make";
    clearer.Clear(dependFile.c_str());

    // Remove the internal dependency check file to force
    // regeneration.
    std::string internalDependFile = dir + "/depend.internal";
    cmSystemTools::RemoveFile(internalDependFile.c_str());
    }
}


void cmLocalUnixMakefileGenerator3
::WriteDependLanguageInfo(std::ostream& cmakefileStream, cmTarget &target)
{
  ImplicitDependLanguageMap const& implicitLangs =
    this->GetImplicitDepends(target);

  // list the languages
  cmakefileStream
    << "# The set of languages for which implicit dependencies are needed:\n";
  cmakefileStream
    << "SET(CMAKE_DEPENDS_LANGUAGES\n";
  for(ImplicitDependLanguageMap::const_iterator
        l = implicitLangs.begin(); l != implicitLangs.end(); ++l)
    {
    cmakefileStream << "  \"" << l->first.c_str() << "\"\n";
    }
  cmakefileStream << "  )\n";

  // now list the files for each language
  cmakefileStream
    << "# The set of files for implicit dependencies of each language:\n";
  for(ImplicitDependLanguageMap::const_iterator
        l = implicitLangs.begin(); l != implicitLangs.end(); ++l)
    {
    cmakefileStream
      << "SET(CMAKE_DEPENDS_CHECK_" << l->first.c_str() << "\n";
    ImplicitDependFileMap const& implicitPairs = l->second;

    // for each file pair
    for(ImplicitDependFileMap::const_iterator pi = implicitPairs.begin();
        pi != implicitPairs.end(); ++pi)
      {
      cmakefileStream << "  \"" << pi->second << "\" ";
      cmakefileStream << "\"" << pi->first << "\"\n";
      }
    cmakefileStream << "  )\n";
    }
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::GetObjectFileName(cmTarget& target,
                    const cmSourceFile& source,
                    std::string* nameWithoutTargetDir)
{
  if(const char* fileTargetDirectory =
     source.GetProperty("MACOSX_PACKAGE_LOCATION"))
    {
    // Special handling for OSX package files.
    std::string objectName = this->GetObjectFileNameWithoutTarget(source, 0);
    if(nameWithoutTargetDir)
      {
      *nameWithoutTargetDir = objectName;
      }
    objectName = cmSystemTools::GetFilenameName(objectName.c_str());
    std::string targetName;
    std::string targetNameReal;
    std::string targetNameImport;
    std::string targetNamePDB;
    target.GetExecutableNames(targetName, targetNameReal, targetNameImport,
                              targetNamePDB, this->ConfigurationName.c_str());
    std::string obj;

    // Construct the full path version of the names.
    //
    // If target is a MACOSX_BUNDLE target, then the package location is
    // relative to "${targetDir}/${targetName}.app/Contents"... else it is
    // relative to "${targetDir}"...
    //
    obj = target.GetDirectory();
    obj += "/";
    if ( target.GetPropertyAsBool("MACOSX_BUNDLE") )
      {
      obj += targetName + ".app/Contents/";
      }
    else
      {
      // Emit warning here...? MACOSX_PACKAGE_LOCATION is "most useful" in a
      // MACOSX_BUNDLE...
      }
    obj += fileTargetDirectory;

    // Object names are specified relative to the current build dir.
    obj = this->Convert(obj.c_str(), START_OUTPUT);
    obj += "/";
    obj += objectName;
    return obj;
    }
  else
    {
    // Start with the target directory.
    std::string obj = this->GetTargetDirectory(target);
    obj += "/";

    // Get the object file name without the target directory.
    std::string::size_type dir_len = 0;
    dir_len += strlen(this->Makefile->GetCurrentOutputDirectory());
    dir_len += 1;
    dir_len += obj.size();
    std::string objectName =
      this->GetObjectFileNameWithoutTarget(source, dir_len);
    if(nameWithoutTargetDir)
      {
      *nameWithoutTargetDir = objectName;
      }

    // Append the object name to the target directory.
    obj += objectName;
    return obj;
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::WriteDisclaimer(std::ostream& os)
{
  os
    << "# CMAKE generated file: DO NOT EDIT!\n"
    << "# Generated by \"" << this->GlobalGenerator->GetName() << "\""
    << " Generator, CMake Version "
    << cmVersion::GetMajorVersion() << "."
    << cmVersion::GetMinorVersion() << "\n\n";
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::GetRecursiveMakeCall(const char *makefile, const char* tgt)
{
  // Call make on the given file.
  std::string cmd;
  cmd += "$(MAKE) -f ";
  cmd += this->Convert(makefile,NONE,SHELL);
  cmd += " ";
  
  // Passg down verbosity level.
  if(this->GetMakeSilentFlag().size())
    {
    cmd += this->GetMakeSilentFlag();
    cmd += " ";
    }

  // Most unix makes will pass the command line flags to make down to
  // sub-invoked makes via an environment variable.  However, some
  // makes do not support that, so you have to pass the flags
  // explicitly.
  if(this->GetPassMakeflags())
    {
    cmd += "-$(MAKEFLAGS) ";
    }

  // Add the target.
  if (tgt && tgt[0] != '\0')
    {
    // The make target is always relative to the top of the build tree.
    std::string tgt2 = this->Convert(tgt, HOME_OUTPUT);

    // The target may have been written with windows paths.
    cmSystemTools::ConvertToOutputSlashes(tgt2);

    // Escape one extra time if the make tool requires it.
    if(this->MakeCommandEscapeTargetTwice)
      {
      tgt2 = this->EscapeForShell(tgt2.c_str(), true, false);
      }

    // The target name is now a string that should be passed verbatim
    // on the command line.
    cmd += this->EscapeForShell(tgt2.c_str(), true, false);
    }
  return cmd;
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::WriteDivider(std::ostream& os)
{
  os
    << "#======================================"
    << "=======================================\n";
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteCMakeArgument(std::ostream& os, const char* s)
{
  // Write the given string to the stream with escaping to get it back
  // into CMake through the lexical scanner.
  os << "\"";
  for(const char* c = s; *c; ++c)
    {
    if(*c == '\\')
      {
      os << "\\\\";
      }
    else if(*c == '"')
      {
      os << "\\\"";
      }
    else
      {
      os << *c;
      }
    }
  os << "\"";
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3::ConvertToQuotedOutputPath(const char* p)
{
  
  // Split the path into its components.
  std::vector<std::string> components;
  cmSystemTools::SplitPath(p, components);

  // Return an empty path if there are no components.
  if(components.empty())
    {
    return "\"\"";
    }

  // Choose a slash direction and fix root component.
  const char* slash = "/";
#if defined(_WIN32) && !defined(__CYGWIN__)
   if(!cmSystemTools::GetForceUnixPaths())
     {
     slash = "\\";
     for(std::string::iterator i = components[0].begin();
       i != components[0].end(); ++i)
       {
       if(*i == '/')
         {
         *i = '\\';
         }
       }
     }
#endif

  // Begin the quoted result with the root component.
  std::string result = "\"";
  result += components[0];

  // Now add the rest of the components separated by the proper slash
  // direction for this platform.
  bool first = true;
  for(unsigned int i=1; i < components.size(); ++i)
    {
    // Only the last component can be empty to avoid double slashes.
    if(components[i].length() > 0 || (i == (components.size()-1)))
      {
      if(!first)
        {
        result += slash;
        }
      result += components[i];
      first = false;
      }
    }

  // Close the quoted result.
  result += "\"";

  return result;
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::GetTargetDirectory(cmTarget const& target) const
{
  std::string dir = cmake::GetCMakeFilesDirectoryPostSlash();
  dir += target.GetName();
  dir += ".dir";
  return dir;
}

//----------------------------------------------------------------------------
cmLocalUnixMakefileGenerator3::ImplicitDependLanguageMap const&
cmLocalUnixMakefileGenerator3::GetImplicitDepends(cmTarget const& tgt)
{
  return this->ImplicitDepends[tgt.GetName()];
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3::AddImplicitDepends(cmTarget const& tgt,
                                                  const char* lang,
                                                  const char* obj,
                                                  const char* src)
{
  this->ImplicitDepends[tgt.GetName()][lang][obj] = src;
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3
::CreateCDCommand(std::vector<std::string>& commands, const char *tgtDir,
                  const char *retDir)
{
  // do we need to cd?
  if (!strcmp(tgtDir,retDir))
    {
    return;
    }
  
  if(!this->UnixCD)
    {
    // On Windows we must perform each step separately and then change
    // back because the shell keeps the working directory between
    // commands.
    std::string cmd = "cd ";
    cmd += this->ConvertToOutputForExisting(tgtDir);
    commands.insert(commands.begin(),cmd);
    
    // Change back to the starting directory.  Any trailing slash must be
    // removed to avoid problems with Borland Make.
    std::string back = retDir;
    if(back.size() && back[back.size()-1] == '/')
      {
      back = back.substr(0, back.size()-1);
      }
    cmd = "cd ";
    cmd += this->ConvertToOutputForExisting(back.c_str());
    commands.push_back(cmd);
    }
  else
    {
    // On UNIX we must construct a single shell command to change
    // directory and build because make resets the directory between
    // each command.
    std::vector<std::string>::iterator i = commands.begin();
    for (; i != commands.end(); ++i)
      {
      std::string cmd = "cd ";
      cmd += this->ConvertToOutputForExisting(tgtDir);
      cmd += " && ";
      cmd += *i;
      *i = cmd;
      }
    }
}


void cmLocalUnixMakefileGenerator3
::GetTargetObjectFileDirectories(cmTarget* target,
                                 std::vector<std::string>& dirs)
{
  std::string dir = this->Makefile->GetCurrentOutputDirectory();
  dir += "/";
  dir += this->GetTargetDirectory(*target);
  dirs.push_back(dir);
}
