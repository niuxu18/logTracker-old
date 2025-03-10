/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCoreTryCompile.h"

#include "cmAlgorithms.h"
#include "cmExportTryCompileFileGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmOutputConverter.h"
#include "cmake.h"
#include <cmsys/Directory.hxx>

#include <assert.h>

static std::string const kCMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN =
  "CMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN";
static std::string const kCMAKE_C_COMPILER_TARGET = "CMAKE_C_COMPILER_TARGET";
static std::string const kCMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN =
  "CMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN";
static std::string const kCMAKE_CXX_COMPILER_TARGET =
  "CMAKE_CXX_COMPILER_TARGET";
static std::string const kCMAKE_ENABLE_EXPORTS = "CMAKE_ENABLE_EXPORTS";
static std::string const kCMAKE_LINK_SEARCH_END_STATIC =
  "CMAKE_LINK_SEARCH_END_STATIC";
static std::string const kCMAKE_LINK_SEARCH_START_STATIC =
  "CMAKE_LINK_SEARCH_START_STATIC";
static std::string const kCMAKE_OSX_ARCHITECTURES = "CMAKE_OSX_ARCHITECTURES";
static std::string const kCMAKE_OSX_DEPLOYMENT_TARGET =
  "CMAKE_OSX_DEPLOYMENT_TARGET";
static std::string const kCMAKE_OSX_SYSROOT = "CMAKE_OSX_SYSROOT";
static std::string const kCMAKE_POSITION_INDEPENDENT_CODE =
  "CMAKE_POSITION_INDEPENDENT_CODE";
static std::string const kCMAKE_SYSROOT = "CMAKE_SYSROOT";
static std::string const kCMAKE_TRY_COMPILE_OSX_ARCHITECTURES =
  "CMAKE_TRY_COMPILE_OSX_ARCHITECTURES";

int cmCoreTryCompile::TryCompileCode(std::vector<std::string> const& argv,
                                     bool isTryRun)
{
  this->BinaryDirectory = argv[1].c_str();
  this->OutputFile = "";
  // which signature were we called with ?
  this->SrcFileSignature = true;

  cmState::TargetType targetType = cmState::EXECUTABLE;
  const char* tt =
    this->Makefile->GetDefinition("CMAKE_TRY_COMPILE_TARGET_TYPE");
  if (!isTryRun && tt && *tt) {
    if (strcmp(tt, cmState::GetTargetTypeName(cmState::EXECUTABLE)) == 0) {
      targetType = cmState::EXECUTABLE;
    } else if (strcmp(tt, cmState::GetTargetTypeName(
                            cmState::STATIC_LIBRARY)) == 0) {
      targetType = cmState::STATIC_LIBRARY;
    } else {
      this->Makefile->IssueMessage(
        cmake::FATAL_ERROR, std::string("Invalid value '") + tt +
          "' for "
          "CMAKE_TRY_COMPILE_TARGET_TYPE.  Only "
          "'" +
          cmState::GetTargetTypeName(cmState::EXECUTABLE) + "' and "
                                                            "'" +
          cmState::GetTargetTypeName(cmState::STATIC_LIBRARY) +
          "' "
          "are allowed.");
      return -1;
    }
  }

  const char* sourceDirectory = argv[2].c_str();
  const char* projectName = 0;
  std::string targetName;
  std::vector<std::string> cmakeFlags(1, "CMAKE_FLAGS"); // fake argv[0]
  std::vector<std::string> compileDefs;
  std::string outputVariable;
  std::string copyFile;
  std::string copyFileError;
  std::vector<std::string> targets;
  std::string libsToLink = " ";
  bool useOldLinkLibs = true;
  char targetNameBuf[64];
  bool didOutputVariable = false;
  bool didCopyFile = false;
  bool didCopyFileError = false;
  bool useSources = argv[2] == "SOURCES";
  std::vector<std::string> sources;

  enum Doing
  {
    DoingNone,
    DoingCMakeFlags,
    DoingCompileDefinitions,
    DoingLinkLibraries,
    DoingOutputVariable,
    DoingCopyFile,
    DoingCopyFileError,
    DoingSources
  };
  Doing doing = useSources ? DoingSources : DoingNone;
  for (size_t i = 3; i < argv.size(); ++i) {
    if (argv[i] == "CMAKE_FLAGS") {
      doing = DoingCMakeFlags;
    } else if (argv[i] == "COMPILE_DEFINITIONS") {
      doing = DoingCompileDefinitions;
    } else if (argv[i] == "LINK_LIBRARIES") {
      doing = DoingLinkLibraries;
      useOldLinkLibs = false;
    } else if (argv[i] == "OUTPUT_VARIABLE") {
      doing = DoingOutputVariable;
      didOutputVariable = true;
    } else if (argv[i] == "COPY_FILE") {
      doing = DoingCopyFile;
      didCopyFile = true;
    } else if (argv[i] == "COPY_FILE_ERROR") {
      doing = DoingCopyFileError;
      didCopyFileError = true;
    } else if (doing == DoingCMakeFlags) {
      cmakeFlags.push_back(argv[i]);
    } else if (doing == DoingCompileDefinitions) {
      compileDefs.push_back(argv[i]);
    } else if (doing == DoingLinkLibraries) {
      libsToLink += "\"" + cmSystemTools::TrimWhitespace(argv[i]) + "\" ";
      if (cmTarget* tgt = this->Makefile->FindTargetToUse(argv[i])) {
        switch (tgt->GetType()) {
          case cmState::SHARED_LIBRARY:
          case cmState::STATIC_LIBRARY:
          case cmState::INTERFACE_LIBRARY:
          case cmState::UNKNOWN_LIBRARY:
            break;
          case cmState::EXECUTABLE:
            if (tgt->IsExecutableWithExports()) {
              break;
            }
          default:
            this->Makefile->IssueMessage(
              cmake::FATAL_ERROR,
              "Only libraries may be used as try_compile or try_run IMPORTED "
              "LINK_LIBRARIES.  Got " +
                std::string(tgt->GetName()) + " of "
                                              "type " +
                cmState::GetTargetTypeName(tgt->GetType()) + ".");
            return -1;
        }
        if (tgt->IsImported()) {
          targets.push_back(argv[i]);
        }
      }
    } else if (doing == DoingOutputVariable) {
      outputVariable = argv[i].c_str();
      doing = DoingNone;
    } else if (doing == DoingCopyFile) {
      copyFile = argv[i].c_str();
      doing = DoingNone;
    } else if (doing == DoingCopyFileError) {
      copyFileError = argv[i].c_str();
      doing = DoingNone;
    } else if (doing == DoingSources) {
      sources.push_back(argv[i]);
    } else if (i == 3) {
      this->SrcFileSignature = false;
      projectName = argv[i].c_str();
    } else if (i == 4 && !this->SrcFileSignature) {
      targetName = argv[i].c_str();
    } else {
      std::ostringstream m;
      m << "try_compile given unknown argument \"" << argv[i] << "\".";
      this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, m.str());
    }
  }

  if (didCopyFile && copyFile.empty()) {
    this->Makefile->IssueMessage(cmake::FATAL_ERROR,
                                 "COPY_FILE must be followed by a file path");
    return -1;
  }

  if (didCopyFileError && copyFileError.empty()) {
    this->Makefile->IssueMessage(
      cmake::FATAL_ERROR,
      "COPY_FILE_ERROR must be followed by a variable name");
    return -1;
  }

  if (didCopyFileError && !didCopyFile) {
    this->Makefile->IssueMessage(
      cmake::FATAL_ERROR, "COPY_FILE_ERROR may be used only with COPY_FILE");
    return -1;
  }

  if (didOutputVariable && outputVariable.empty()) {
    this->Makefile->IssueMessage(
      cmake::FATAL_ERROR,
      "OUTPUT_VARIABLE must be followed by a variable name");
    return -1;
  }

  if (useSources && sources.empty()) {
    this->Makefile->IssueMessage(
      cmake::FATAL_ERROR,
      "SOURCES must be followed by at least one source file");
    return -1;
  }

  // compute the binary dir when TRY_COMPILE is called with a src file
  // signature
  if (this->SrcFileSignature) {
    this->BinaryDirectory += cmake::GetCMakeFilesDirectory();
    this->BinaryDirectory += "/CMakeTmp";
  } else {
    // only valid for srcfile signatures
    if (!compileDefs.empty()) {
      this->Makefile->IssueMessage(
        cmake::FATAL_ERROR,
        "COMPILE_DEFINITIONS specified on a srcdir type TRY_COMPILE");
      return -1;
    }
    if (!copyFile.empty()) {
      this->Makefile->IssueMessage(
        cmake::FATAL_ERROR,
        "COPY_FILE specified on a srcdir type TRY_COMPILE");
      return -1;
    }
  }
  // make sure the binary directory exists
  cmSystemTools::MakeDirectory(this->BinaryDirectory.c_str());

  // do not allow recursive try Compiles
  if (this->BinaryDirectory == this->Makefile->GetHomeOutputDirectory()) {
    std::ostringstream e;
    e << "Attempt at a recursive or nested TRY_COMPILE in directory\n"
      << "  " << this->BinaryDirectory << "\n";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    return -1;
  }

  std::string outFileName = this->BinaryDirectory + "/CMakeLists.txt";
  // which signature are we using? If we are using var srcfile bindir
  if (this->SrcFileSignature) {
    // remove any CMakeCache.txt files so we will have a clean test
    std::string ccFile = this->BinaryDirectory + "/CMakeCache.txt";
    cmSystemTools::RemoveFile(ccFile);

    // Choose sources.
    if (!useSources) {
      sources.push_back(argv[2]);
    }

    // Detect languages to enable.
    cmGlobalGenerator* gg = this->Makefile->GetGlobalGenerator();
    std::set<std::string> testLangs;
    for (std::vector<std::string>::iterator si = sources.begin();
         si != sources.end(); ++si) {
      std::string ext = cmSystemTools::GetFilenameLastExtension(*si);
      std::string lang = gg->GetLanguageFromExtension(ext.c_str());
      if (!lang.empty()) {
        testLangs.insert(lang);
      } else {
        std::ostringstream err;
        err << "Unknown extension \"" << ext << "\" for file\n"
            << "  " << *si << "\n"
            << "try_compile() works only for enabled languages.  "
            << "Currently these are:\n  ";
        std::vector<std::string> langs;
        gg->GetEnabledLanguages(langs);
        err << cmJoin(langs, " ");
        err << "\nSee project() command to enable other languages.";
        this->Makefile->IssueMessage(cmake::FATAL_ERROR, err.str());
        return -1;
      }
    }

    std::string const tcConfig =
      this->Makefile->GetSafeDefinition("CMAKE_TRY_COMPILE_CONFIGURATION");

    // we need to create a directory and CMakeLists file etc...
    // first create the directories
    sourceDirectory = this->BinaryDirectory.c_str();

    // now create a CMakeLists.txt file in that directory
    FILE* fout = cmsys::SystemTools::Fopen(outFileName, "w");
    if (!fout) {
      std::ostringstream e;
      /* clang-format off */
      e << "Failed to open\n"
        << "  " << outFileName << "\n"
        << cmSystemTools::GetLastSystemError();
      /* clang-format on */
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
      return -1;
    }

    const char* def = this->Makefile->GetDefinition("CMAKE_MODULE_PATH");
    fprintf(fout, "cmake_minimum_required(VERSION %u.%u.%u.%u)\n",
            cmVersion::GetMajorVersion(), cmVersion::GetMinorVersion(),
            cmVersion::GetPatchVersion(), cmVersion::GetTweakVersion());
    if (def) {
      fprintf(fout, "set(CMAKE_MODULE_PATH \"%s\")\n", def);
    }

    std::string projectLangs;
    for (std::set<std::string>::iterator li = testLangs.begin();
         li != testLangs.end(); ++li) {
      projectLangs += " " + *li;
      std::string rulesOverrideBase = "CMAKE_USER_MAKE_RULES_OVERRIDE";
      std::string rulesOverrideLang = rulesOverrideBase + "_" + *li;
      if (const char* rulesOverridePath =
            this->Makefile->GetDefinition(rulesOverrideLang)) {
        fprintf(fout, "set(%s \"%s\")\n", rulesOverrideLang.c_str(),
                rulesOverridePath);
      } else if (const char* rulesOverridePath2 =
                   this->Makefile->GetDefinition(rulesOverrideBase)) {
        fprintf(fout, "set(%s \"%s\")\n", rulesOverrideBase.c_str(),
                rulesOverridePath2);
      }
    }
    fprintf(fout, "project(CMAKE_TRY_COMPILE%s)\n", projectLangs.c_str());
    fprintf(fout, "set(CMAKE_VERBOSE_MAKEFILE 1)\n");
    for (std::set<std::string>::iterator li = testLangs.begin();
         li != testLangs.end(); ++li) {
      std::string langFlags = "CMAKE_" + *li + "_FLAGS";
      const char* flags = this->Makefile->GetDefinition(langFlags);
      fprintf(fout, "set(CMAKE_%s_FLAGS %s)\n", li->c_str(),
              cmOutputConverter::EscapeForCMake(flags ? flags : "").c_str());
      fprintf(fout, "set(CMAKE_%s_FLAGS \"${CMAKE_%s_FLAGS}"
                    " ${COMPILE_DEFINITIONS}\")\n",
              li->c_str(), li->c_str());
      static std::string const cfgDefault = "DEBUG";
      std::string const cfg =
        !tcConfig.empty() ? cmSystemTools::UpperCase(tcConfig) : cfgDefault;
      std::string const langFlagsCfg = "CMAKE_" + *li + "_FLAGS_" + cfg;
      const char* flagsCfg = this->Makefile->GetDefinition(langFlagsCfg);
      fprintf(
        fout, "set(%s %s)\n", langFlagsCfg.c_str(),
        cmOutputConverter::EscapeForCMake(flagsCfg ? flagsCfg : "").c_str());
    }
    switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0056)) {
      case cmPolicies::WARN:
        if (this->Makefile->PolicyOptionalWarningEnabled(
              "CMAKE_POLICY_WARNING_CMP0056")) {
          std::ostringstream w;
          /* clang-format off */
          w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0056) << "\n"
            "For compatibility with older versions of CMake, try_compile "
            "is not honoring caller link flags (e.g. CMAKE_EXE_LINKER_FLAGS) "
            "in the test project."
            ;
          /* clang-format on */
          this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, w.str());
        }
      case cmPolicies::OLD:
        // OLD behavior is to do nothing.
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        this->Makefile->IssueMessage(
          cmake::FATAL_ERROR,
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0056));
      case cmPolicies::NEW:
        // NEW behavior is to pass linker flags.
        {
          const char* exeLinkFlags =
            this->Makefile->GetDefinition("CMAKE_EXE_LINKER_FLAGS");
          fprintf(
            fout, "set(CMAKE_EXE_LINKER_FLAGS %s)\n",
            cmOutputConverter::EscapeForCMake(exeLinkFlags ? exeLinkFlags : "")
              .c_str());
        }
        break;
    }
    fprintf(fout, "set(CMAKE_EXE_LINKER_FLAGS \"${CMAKE_EXE_LINKER_FLAGS}"
                  " ${EXE_LINKER_FLAGS}\")\n");
    fprintf(fout, "include_directories(${INCLUDE_DIRECTORIES})\n");
    fprintf(fout, "set(CMAKE_SUPPRESS_REGENERATION 1)\n");
    fprintf(fout, "link_directories(${LINK_DIRECTORIES})\n");
    // handle any compile flags we need to pass on
    if (!compileDefs.empty()) {
      fprintf(fout, "add_definitions(%s)\n", cmJoin(compileDefs, " ").c_str());
    }

    /* Use a random file name to avoid rapid creation and deletion
       of the same executable name (some filesystems fail on that).  */
    sprintf(targetNameBuf, "cmTC_%05x", cmSystemTools::RandomSeed() & 0xFFFFF);
    targetName = targetNameBuf;

    if (!targets.empty()) {
      std::string fname = "/" + std::string(targetName) + "Targets.cmake";
      cmExportTryCompileFileGenerator tcfg(gg, targets, this->Makefile);
      tcfg.SetExportFile((this->BinaryDirectory + fname).c_str());
      tcfg.SetConfig(tcConfig);

      if (!tcfg.GenerateImportFile()) {
        this->Makefile->IssueMessage(cmake::FATAL_ERROR,
                                     "could not write export file.");
        fclose(fout);
        return -1;
      }
      fprintf(fout, "\ninclude(\"${CMAKE_CURRENT_LIST_DIR}/%s\")\n\n",
              fname.c_str());
    }

    // Forward a set of variables to the inner project cache.
    {
      std::set<std::string> vars;
      vars.insert(kCMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN);
      vars.insert(kCMAKE_C_COMPILER_TARGET);
      vars.insert(kCMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN);
      vars.insert(kCMAKE_CXX_COMPILER_TARGET);
      vars.insert(kCMAKE_ENABLE_EXPORTS);
      vars.insert(kCMAKE_LINK_SEARCH_END_STATIC);
      vars.insert(kCMAKE_LINK_SEARCH_START_STATIC);
      vars.insert(kCMAKE_OSX_ARCHITECTURES);
      vars.insert(kCMAKE_OSX_DEPLOYMENT_TARGET);
      vars.insert(kCMAKE_OSX_SYSROOT);
      vars.insert(kCMAKE_POSITION_INDEPENDENT_CODE);
      vars.insert(kCMAKE_SYSROOT);

      /* for the TRY_COMPILEs we want to be able to specify the architecture.
         So the user can set CMAKE_OSX_ARCHITECTURES to i386;ppc and then set
         CMAKE_TRY_COMPILE_OSX_ARCHITECTURES first to i386 and then to ppc to
         have the tests run for each specific architecture. Since
         cmLocalGenerator doesn't allow building for "the other"
         architecture only via CMAKE_OSX_ARCHITECTURES.
         */
      if (const char* tcArchs = this->Makefile->GetDefinition(
            kCMAKE_TRY_COMPILE_OSX_ARCHITECTURES)) {
        vars.erase(kCMAKE_OSX_ARCHITECTURES);
        std::string flag = "-DCMAKE_OSX_ARCHITECTURES=" + std::string(tcArchs);
        cmakeFlags.push_back(flag);
      }

      for (std::set<std::string>::iterator vi = vars.begin(); vi != vars.end();
           ++vi) {
        std::string const& var = *vi;
        if (const char* val = this->Makefile->GetDefinition(var)) {
          std::string flag = "-D" + var + "=" + val;
          cmakeFlags.push_back(flag);
        }
      }
    }

    /* Set the appropriate policy information for ENABLE_EXPORTS */
    fprintf(fout, "cmake_policy(SET CMP0065 %s)\n",
            this->Makefile->GetPolicyStatus(cmPolicies::CMP0065) ==
                cmPolicies::NEW
              ? "NEW"
              : "OLD");

    if (targetType == cmState::EXECUTABLE) {
      /* Put the executable at a known location (for COPY_FILE).  */
      fprintf(fout, "set(CMAKE_RUNTIME_OUTPUT_DIRECTORY \"%s\")\n",
              this->BinaryDirectory.c_str());
      /* Create the actual executable.  */
      fprintf(fout, "add_executable(%s", targetName.c_str());
    } else // if (targetType == cmState::STATIC_LIBRARY)
    {
      /* Put the static library at a known location (for COPY_FILE).  */
      fprintf(fout, "set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY \"%s\")\n",
              this->BinaryDirectory.c_str());
      /* Create the actual static library.  */
      fprintf(fout, "add_library(%s STATIC", targetName.c_str());
    }
    for (std::vector<std::string>::iterator si = sources.begin();
         si != sources.end(); ++si) {
      fprintf(fout, " \"%s\"", si->c_str());

      // Add dependencies on any non-temporary sources.
      if (si->find("CMakeTmp") == si->npos) {
        this->Makefile->AddCMakeDependFile(*si);
      }
    }
    fprintf(fout, ")\n");
    if (useOldLinkLibs) {
      fprintf(fout, "target_link_libraries(%s ${LINK_LIBRARIES})\n",
              targetName.c_str());
    } else {
      fprintf(fout, "target_link_libraries(%s %s)\n", targetName.c_str(),
              libsToLink.c_str());
    }
    fclose(fout);
    projectName = "CMAKE_TRY_COMPILE";
  }

  bool erroroc = cmSystemTools::GetErrorOccuredFlag();
  cmSystemTools::ResetErrorOccuredFlag();
  std::string output;
  // actually do the try compile now that everything is setup
  int res = this->Makefile->TryCompile(
    sourceDirectory, this->BinaryDirectory, projectName, targetName,
    this->SrcFileSignature, &cmakeFlags, output);
  if (erroroc) {
    cmSystemTools::SetErrorOccured();
  }

  // set the result var to the return value to indicate success or failure
  this->Makefile->AddCacheDefinition(argv[0], (res == 0 ? "TRUE" : "FALSE"),
                                     "Result of TRY_COMPILE",
                                     cmState::INTERNAL);

  if (!outputVariable.empty()) {
    this->Makefile->AddDefinition(outputVariable, output.c_str());
  }

  if (this->SrcFileSignature) {
    std::string copyFileErrorMessage;
    this->FindOutputFile(targetName, targetType);

    if ((res == 0) && !copyFile.empty()) {
      if (this->OutputFile.empty() ||
          !cmSystemTools::CopyFileAlways(this->OutputFile, copyFile)) {
        std::ostringstream emsg;
        /* clang-format off */
        emsg << "Cannot copy output executable\n"
             << "  '" << this->OutputFile << "'\n"
             << "to destination specified by COPY_FILE:\n"
             << "  '" << copyFile << "'\n";
        /* clang-format on */
        if (!this->FindErrorMessage.empty()) {
          emsg << this->FindErrorMessage.c_str();
        }
        if (copyFileError.empty()) {
          this->Makefile->IssueMessage(cmake::FATAL_ERROR, emsg.str());
          return -1;
        } else {
          copyFileErrorMessage = emsg.str();
        }
      }
    }

    if (!copyFileError.empty()) {
      this->Makefile->AddDefinition(copyFileError,
                                    copyFileErrorMessage.c_str());
    }
  }
  return res;
}

void cmCoreTryCompile::CleanupFiles(const char* binDir)
{
  if (!binDir) {
    return;
  }

  std::string bdir = binDir;
  if (bdir.find("CMakeTmp") == std::string::npos) {
    cmSystemTools::Error(
      "TRY_COMPILE attempt to remove -rf directory that does not contain "
      "CMakeTmp:",
      binDir);
    return;
  }

  cmsys::Directory dir;
  dir.Load(binDir);
  size_t fileNum;
  std::set<std::string> deletedFiles;
  for (fileNum = 0; fileNum < dir.GetNumberOfFiles(); ++fileNum) {
    if (strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)), ".") &&
        strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)), "..")) {

      if (deletedFiles.find(dir.GetFile(
            static_cast<unsigned long>(fileNum))) == deletedFiles.end()) {
        deletedFiles.insert(dir.GetFile(static_cast<unsigned long>(fileNum)));
        std::string fullPath = binDir;
        fullPath += "/";
        fullPath += dir.GetFile(static_cast<unsigned long>(fileNum));
        if (cmSystemTools::FileIsDirectory(fullPath)) {
          this->CleanupFiles(fullPath.c_str());
          cmSystemTools::RemoveADirectory(fullPath);
        } else {
#ifdef _WIN32
          // Sometimes anti-virus software hangs on to new files so we
          // cannot delete them immediately.  Try a few times.
          cmSystemTools::WindowsFileRetry retry =
            cmSystemTools::GetWindowsFileRetry();
          while (!cmSystemTools::RemoveFile(fullPath.c_str()) &&
                 --retry.Count &&
                 cmSystemTools::FileExists(fullPath.c_str())) {
            cmSystemTools::Delay(retry.Delay);
          }
          if (retry.Count == 0)
#else
          if (!cmSystemTools::RemoveFile(fullPath))
#endif
          {
            std::string m = "Remove failed on file: " + fullPath;
            cmSystemTools::ReportLastSystemError(m.c_str());
          }
        }
      }
    }
  }
}

void cmCoreTryCompile::FindOutputFile(const std::string& targetName,
                                      cmState::TargetType targetType)
{
  this->FindErrorMessage = "";
  this->OutputFile = "";
  std::string tmpOutputFile = "/";
  if (targetType == cmState::EXECUTABLE) {
    tmpOutputFile += targetName;
    tmpOutputFile +=
      this->Makefile->GetSafeDefinition("CMAKE_EXECUTABLE_SUFFIX");
  } else // if (targetType == cmState::STATIC_LIBRARY)
  {
    tmpOutputFile +=
      this->Makefile->GetSafeDefinition("CMAKE_STATIC_LIBRARY_PREFIX");
    tmpOutputFile += targetName;
    tmpOutputFile +=
      this->Makefile->GetSafeDefinition("CMAKE_STATIC_LIBRARY_SUFFIX");
  }

  // a list of directories where to search for the compilation result
  // at first directly in the binary dir
  std::vector<std::string> searchDirs;
  searchDirs.push_back("");

  const char* config =
    this->Makefile->GetDefinition("CMAKE_TRY_COMPILE_CONFIGURATION");
  // if a config was specified try that first
  if (config && config[0]) {
    std::string tmp = "/";
    tmp += config;
    searchDirs.push_back(tmp);
  }
  searchDirs.push_back("/Debug");
#if defined(__APPLE__)
  std::string app = "/Debug/" + targetName + ".app";
  searchDirs.push_back(app);
#endif
  searchDirs.push_back("/Development");

  for (std::vector<std::string>::const_iterator it = searchDirs.begin();
       it != searchDirs.end(); ++it) {
    std::string command = this->BinaryDirectory;
    command += *it;
    command += tmpOutputFile;
    if (cmSystemTools::FileExists(command.c_str())) {
      this->OutputFile = cmSystemTools::CollapseFullPath(command);
      return;
    }
  }

  std::ostringstream emsg;
  emsg << "Unable to find the executable at any of:\n";
  emsg << cmWrap("  " + this->BinaryDirectory, searchDirs, tmpOutputFile, "\n")
       << "\n";
  this->FindErrorMessage = emsg.str();
  return;
}
