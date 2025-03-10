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

#include "cmCTestScriptHandler.h"

#include "cmCTest.h"
#include "cmake.h"
#include "cmFunctionBlocker.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmGeneratedFileStream.h"

//#include <cmsys/RegularExpression.hxx>
#include <cmsys/Process.h>

// used for sleep
#ifdef _WIN32
#include "windows.h"
#endif

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <float.h>

// needed for sleep
#if !defined(_WIN32)
# include <unistd.h>
#endif

#include "cmCTestBuildCommand.h"
#include "cmCTestConfigureCommand.h"
#include "cmCTestCoverageCommand.h"
#include "cmCTestEmptyBinaryDirectoryCommand.h"
#include "cmCTestMemCheckCommand.h"
#include "cmCTestRunScriptCommand.h"
#include "cmCTestSleepCommand.h"
#include "cmCTestStartCommand.h"
#include "cmCTestSubmitCommand.h"
#include "cmCTestTestCommand.h"
#include "cmCTestUpdateCommand.h"

#define CTEST_INITIAL_CMAKE_OUTPUT_FILE_NAME "CTestInitialCMakeOutput.log"

// used to keep elapsed time up to date
class cmCTestScriptFunctionBlocker : public cmFunctionBlocker
{
public:
  cmCTestScriptFunctionBlocker() {}
  virtual ~cmCTestScriptFunctionBlocker() {}
  virtual bool IsFunctionBlocked(const cmListFileFunction& lff,
                                 cmMakefile &mf);
  //virtual bool ShouldRemove(const cmListFileFunction& lff, cmMakefile &mf);
  //virtual void ScopeEnded(cmMakefile &mf);

  cmCTestScriptHandler* m_CTestScriptHandler;
};

// simply update the time and don't block anything
bool cmCTestScriptFunctionBlocker::
IsFunctionBlocked(const cmListFileFunction& , cmMakefile &)
{
  m_CTestScriptHandler->UpdateElapsedTime();
  return false;
}

//----------------------------------------------------------------------
cmCTestScriptHandler::cmCTestScriptHandler()
{
  m_Backup = false;
  m_EmptyBinDir = false;
  m_EmptyBinDirOnce = false;
  m_Makefile = 0;
  m_LocalGenerator = 0;
  m_CMake = 0;
  m_GlobalGenerator = 0;

  m_ScriptStartTime = 0;

  // the *60 is becuase the settings are in minutes but GetTime is seconds
  m_MinimumInterval = 30*60;
  m_ContinuousDuration = -1;
}

//----------------------------------------------------------------------
void cmCTestScriptHandler::Initialize()
{
  this->Superclass::Initialize();
  m_Backup = false;
  m_EmptyBinDir = false;
  m_EmptyBinDirOnce = false;

  m_SourceDir = "";
  m_BinaryDir = "";
  m_BackupSourceDir = "";
  m_BackupBinaryDir = "";
  m_CTestRoot = "";
  m_CVSCheckOut = "";
  m_CTestCmd = "";
  m_CVSCmd = "";
  m_CTestEnv = "";
  m_InitCache = "";
  m_CMakeCmd = "";
  m_CMOutFile = "";
  m_ExtraUpdates.clear();

  m_MinimumInterval = 20*60;
  m_ContinuousDuration = -1;

  // what time in seconds did this script start running
  m_ScriptStartTime = 0;

  m_Makefile = 0;
  if (m_LocalGenerator)
    {
    delete m_LocalGenerator;
    }
  m_LocalGenerator = 0;
  if (m_GlobalGenerator)
    {
    delete m_GlobalGenerator;
    }
  m_GlobalGenerator = 0;
  if (m_CMake)
    {
    delete m_CMake;
    }
}

//----------------------------------------------------------------------
cmCTestScriptHandler::~cmCTestScriptHandler()
{
  // local generator owns the makefile
  m_Makefile = 0;
  if (m_LocalGenerator)
    {
    delete m_LocalGenerator;
    }
  m_LocalGenerator = 0;
  if (m_GlobalGenerator)
    {
    delete m_GlobalGenerator;
    }
  m_GlobalGenerator = 0;
  if (m_CMake)
    {
    delete m_CMake;
    }
}


//----------------------------------------------------------------------
// just adds an argument to the vector
void cmCTestScriptHandler::AddConfigurationScript(const char *script)
{
  m_ConfigurationScripts.push_back(script);
}


//----------------------------------------------------------------------
// the generic entry point for handling scripts, this routine will run all
// the scripts provides a -S arguments
int cmCTestScriptHandler::ProcessHandler()
{
  int res = 0;
  std::vector<cmStdString>::iterator it;
  for ( it = m_ConfigurationScripts.begin();
        it != m_ConfigurationScripts.end();
        it ++ )
    {
    // for each script run it
    res += this->RunConfigurationScript(
      cmSystemTools::CollapseFullPath(it->c_str()));
    }
  if ( res )
    {
    return -1;
    }
  return 0;
}

void cmCTestScriptHandler::UpdateElapsedTime()
{
  if (m_LocalGenerator)
    {
    // set the current elapsed time
    char timeString[20];
    int itime = static_cast<unsigned int>(cmSystemTools::GetTime()
                                          - m_ScriptStartTime);
    sprintf(timeString,"%i",itime);
    m_LocalGenerator->GetMakefile()->AddDefinition("CTEST_ELAPSED_TIME",
                                                   timeString);
    }
}

//----------------------------------------------------------------------
void cmCTestScriptHandler::AddCTestCommand(cmCTestCommand* command)
{
  cmCTestCommand* newCom = command;
  newCom->m_CTest = m_CTest;
  newCom->m_CTestScriptHandler = this;
  m_CMake->AddCommand(newCom);
}

//----------------------------------------------------------------------
// this sets up some variables for thew script to use, creates the required
// cmake instance and generators, and then reads in the script
int cmCTestScriptHandler::ReadInScript(const std::string& total_script_arg)
{
  // if the argument has a , in it then it needs to be broken into the fist
  // argument (which is the script) and the second argument which will be
  // passed into the scripts as S_ARG
  std::string script = total_script_arg;
  std::string script_arg;
  if (total_script_arg.find(",") != std::string::npos)
    {
    script = total_script_arg.substr(0,total_script_arg.find(","));
    script_arg = total_script_arg.substr(total_script_arg.find(",")+1);
    }

  // make sure the file exists
  if (!cmSystemTools::FileExists(script.c_str()))
    {
    cmSystemTools::Error("Cannot find file: ", script.c_str());
    return 1;
    }

  // create a cmake instance to read the configuration script
  // read in the list file to fill the cache
  if (m_CMake)
    {
    delete m_CMake;
    delete m_GlobalGenerator;
    delete m_LocalGenerator;
    }
  m_CMake = new cmake;
  m_CMake->AddCMakePaths(m_CTest->GetCTestExecutable());
  m_GlobalGenerator = new cmGlobalGenerator;
  m_GlobalGenerator->SetCMakeInstance(m_CMake);

  m_LocalGenerator = m_GlobalGenerator->CreateLocalGenerator();
  m_LocalGenerator->SetGlobalGenerator(m_GlobalGenerator);
  m_Makefile = m_LocalGenerator->GetMakefile();

  // set a variable with the path to the current script
  m_Makefile->AddDefinition("CTEST_SCRIPT_DIRECTORY",
                            cmSystemTools::GetFilenamePath(script).c_str());
  m_Makefile->AddDefinition("CTEST_SCRIPT_NAME",
                            cmSystemTools::GetFilenameName(script).c_str());
  m_Makefile->AddDefinition("CTEST_EXECUTABLE_NAME",
                            m_CTest->GetCTestExecutable());
  m_Makefile->AddDefinition("CMAKE_EXECUTABLE_NAME",
                            m_CTest->GetCMakeExecutable());
  m_Makefile->AddDefinition("CTEST_RUN_CURRENT_SCRIPT", true);
  this->UpdateElapsedTime();

  // add any ctest specific commands, probably should have common superclass
  // for ctest commands to clean this up. If a couple more commands are
  // created with the same format lets do that - ken
  this->AddCTestCommand(new cmCTestBuildCommand);
  this->AddCTestCommand(new cmCTestConfigureCommand);
  this->AddCTestCommand(new cmCTestCoverageCommand);
  this->AddCTestCommand(new cmCTestEmptyBinaryDirectoryCommand);
  this->AddCTestCommand(new cmCTestMemCheckCommand);
  this->AddCTestCommand(new cmCTestRunScriptCommand);
  this->AddCTestCommand(new cmCTestSleepCommand);
  this->AddCTestCommand(new cmCTestStartCommand);
  this->AddCTestCommand(new cmCTestSubmitCommand);
  this->AddCTestCommand(new cmCTestTestCommand);
  this->AddCTestCommand(new cmCTestUpdateCommand);

  // add the script arg if defined
  if (script_arg.size())
    {
    m_Makefile->AddDefinition("CTEST_SCRIPT_ARG", script_arg.c_str());
    }

  // always add a function blocker to update the elapsed time
  cmCTestScriptFunctionBlocker *f = new cmCTestScriptFunctionBlocker();
  f->m_CTestScriptHandler = this;
  m_Makefile->AddFunctionBlocker(f);

  // finally read in the script
  if (!m_Makefile->ReadListFile(0, script.c_str()))
    {
    return 2;
    }

  return 0;
}


//----------------------------------------------------------------------
// extract variabels from the script to set ivars
int cmCTestScriptHandler::ExtractVariables()
{
  // Temporary variables
  const char* minInterval;
  const char* contDuration;

  m_SourceDir   = m_Makefile->GetSafeDefinition("CTEST_SOURCE_DIRECTORY");
  m_BinaryDir   = m_Makefile->GetSafeDefinition("CTEST_BINARY_DIRECTORY");
  m_CTestCmd    = m_Makefile->GetSafeDefinition("CTEST_COMMAND");
  m_CVSCheckOut = m_Makefile->GetSafeDefinition("CTEST_CVS_CHECKOUT");
  m_CTestRoot   = m_Makefile->GetSafeDefinition("CTEST_DASHBOARD_ROOT");
  m_CVSCmd      = m_Makefile->GetSafeDefinition("CTEST_CVS_COMMAND");
  m_CTestEnv    = m_Makefile->GetSafeDefinition("CTEST_ENVIRONMENT");
  m_InitCache   = m_Makefile->GetSafeDefinition("CTEST_INITIAL_CACHE");
  m_CMakeCmd    = m_Makefile->GetSafeDefinition("CTEST_CMAKE_COMMAND");
  m_CMOutFile
    = m_Makefile->GetSafeDefinition("CTEST_CMAKE_OUTPUT_FILE_NAME");

  m_Backup      = m_Makefile->IsOn("CTEST_BACKUP_AND_RESTORE");
  m_EmptyBinDir = m_Makefile->IsOn("CTEST_START_WITH_EMPTY_BINARY_DIRECTORY");
  m_EmptyBinDirOnce
    = m_Makefile->IsOn("CTEST_START_WITH_EMPTY_BINARY_DIRECTORY_ONCE");

  minInterval
    = m_Makefile->GetDefinition("CTEST_CONTINUOUS_MINIMUM_INTERVAL");
  contDuration  = m_Makefile->GetDefinition("CTEST_CONTINUOUS_DURATION");

  char updateVar[40];
  int i;
  for (i = 1; i < 10; ++i)
    {
    sprintf(updateVar,"CTEST_EXTRA_UPDATES_%i",i);
    const char *updateVal = m_Makefile->GetDefinition(updateVar);
    if ( updateVal )
      {
      if ( m_CVSCmd.empty() )
        {
        cmSystemTools::Error(updateVar,
          " specified without specifying CTEST_CVS_COMMAND.");
        return 12;
        }
      m_ExtraUpdates.push_back(updateVal);
      }
    }

  // in order to backup and restore we also must have the cvs root
  if (m_Backup && m_CVSCheckOut.empty())
    {
    cmSystemTools::Error(
      "Backup was requested without specifying CTEST_CVS_CHECKOUT.");
    return 3;
    }

  // make sure the required info is here
  if (this->m_SourceDir.empty() ||
      this->m_BinaryDir.empty() ||
      this->m_CTestCmd.empty())
    {
    std::string message = "CTEST_SOURCE_DIRECTORY = ";
    message += (!m_SourceDir.empty()) ? m_SourceDir.c_str() : "(Null)";
    message += "\nCTEST_BINARY_DIRECTORY = ";
    message += (!m_BinaryDir.empty()) ? m_BinaryDir.c_str() : "(Null)";
    message += "\nCTEST_COMMAND = ";
    message += (!m_CTestCmd.empty()) ? m_CTestCmd.c_str() : "(Null)";
    cmSystemTools::Error(
      "Some required settings in the configuration file were missing:\n",
      message.c_str());
    return 4;
    }

  // if the dashboard root isn't specified then we can compute it from the
  // m_SourceDir
  if (m_CTestRoot.empty() )
    {
    m_CTestRoot = cmSystemTools::GetFilenamePath(m_SourceDir).c_str();
    }

  // the script may override the minimum continuous interval
  if (minInterval)
    {
    m_MinimumInterval = 60 * atof(minInterval);
    }
  if (contDuration)
    {
    m_ContinuousDuration = 60.0 * atof(contDuration);
    }


  this->UpdateElapsedTime();

  return 0;
}

//----------------------------------------------------------------------
void cmCTestScriptHandler::SleepInSeconds(unsigned int secondsToWait)
{
#if defined(_WIN32)
        Sleep(1000*secondsToWait);
#else
        sleep(secondsToWait);
#endif
}

//----------------------------------------------------------------------
// run a specific script
int cmCTestScriptHandler::RunConfigurationScript(
  const std::string& total_script_arg)
{
  int result;

  m_ScriptStartTime =
    cmSystemTools::GetTime();

  // read in the script
  result = this->ReadInScript(total_script_arg);
  if (result)
    {
    return result;
    }

  // only run the curent script if we should
  if (m_Makefile && m_Makefile->IsOn("CTEST_RUN_CURRENT_SCRIPT"))
    {
    return this->RunCurrentScript();
    }
  return result;
}

//----------------------------------------------------------------------
int cmCTestScriptHandler::RunCurrentScript()
{
  int result;

  // do not run twice
  m_Makefile->AddDefinition("CTEST_RUN_CURRENT_SCRIPT", false);

  // no popup widows
  cmSystemTools::SetRunCommandHideConsole(true);

  // extract the vars from the cache and store in ivars
  result = this->ExtractVariables();
  if (result)
    {
    return result;
    }

  // set any environment variables
  if (!m_CTestEnv.empty())
    {
    std::vector<std::string> envArgs;
    cmSystemTools::ExpandListArgument(m_CTestEnv.c_str(),envArgs);
    // for each variable/argument do a putenv
    for (unsigned i = 0; i < envArgs.size(); ++i)
      {
      cmSystemTools::PutEnv(envArgs[i].c_str());
      }
    }

  // now that we have done most of the error checking finally run the
  // dashboard, we may be asked to repeatedly run this dashboard, such as
  // for a continuous, do we ned to run it more than once?
  if ( m_ContinuousDuration >= 0 )
    {
    this->UpdateElapsedTime();
    double ending_time  = cmSystemTools::GetTime() + m_ContinuousDuration;
    if (m_EmptyBinDirOnce)
      {
      m_EmptyBinDir = true;
      }
    do
      {
      double interval = cmSystemTools::GetTime();
      result = this->RunConfigurationDashboard();
      interval = cmSystemTools::GetTime() - interval;
      if (interval < m_MinimumInterval)
        {
        this->SleepInSeconds(
          static_cast<unsigned int>(m_MinimumInterval - interval));
        }
      if (m_EmptyBinDirOnce)
        {
        m_EmptyBinDir = false;
        }
      }
    while (cmSystemTools::GetTime() < ending_time);
    }
  // otherwise just run it once
  else
    {
    result = this->RunConfigurationDashboard();
    }

  return result;
}

//----------------------------------------------------------------------
int cmCTestScriptHandler::CheckOutSourceDir()
{
  std::string command;
  std::string output;
  int retVal;
  bool res;

  if (!cmSystemTools::FileExists(m_SourceDir.c_str()) &&
      !m_CVSCheckOut.empty())
    {
    // we must now checkout the src dir
    output = "";
    cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Run cvs: " << m_CVSCheckOut
      << std::endl);
    res = cmSystemTools::RunSingleCommand(m_CVSCheckOut.c_str(), &output,
                                          &retVal, m_CTestRoot.c_str(),
                                          m_HandlerVerbose, 0 /*m_TimeOut*/);
    if (!res || retVal != 0)
      {
      cmSystemTools::Error("Unable to perform cvs checkout:\n",
                           output.c_str());
      return 6;
      }
    }
  return 0;
}

//----------------------------------------------------------------------
int cmCTestScriptHandler::BackupDirectories()
{
  int retVal;

  // compute the backup names
  m_BackupSourceDir = m_SourceDir;
  m_BackupSourceDir += "_CMakeBackup";
  m_BackupBinaryDir = m_BinaryDir;
  m_BackupBinaryDir += "_CMakeBackup";

  // backup the binary and src directories if requested
  if (m_Backup)
    {
    // if for some reason those directories exist then first delete them
    if (cmSystemTools::FileExists(m_BackupSourceDir.c_str()))
      {
      cmSystemTools::RemoveADirectory(m_BackupSourceDir.c_str());
      }
    if (cmSystemTools::FileExists(m_BackupBinaryDir.c_str()))
      {
      cmSystemTools::RemoveADirectory(m_BackupBinaryDir.c_str());
      }

    // first rename the src and binary directories
    rename(m_SourceDir.c_str(), m_BackupSourceDir.c_str());
    rename(m_BinaryDir.c_str(), m_BackupBinaryDir.c_str());

    // we must now checkout the src dir
    retVal = this->CheckOutSourceDir();
    if (retVal)
      {
      this->RestoreBackupDirectories();
      return retVal;
      }
    }

  return 0;
}


//----------------------------------------------------------------------
int cmCTestScriptHandler::PerformExtraUpdates()
{
  std::string command;
  std::string output;
  int retVal;
  bool res;

  // do an initial cvs update as required
  command = m_CVSCmd;
  std::vector<cmStdString>::iterator it;
  for (it = m_ExtraUpdates.begin(); it != m_ExtraUpdates.end(); ++ it )
    {
    std::vector<std::string> cvsArgs;
    cmSystemTools::ExpandListArgument(it->c_str(),cvsArgs);
    if (cvsArgs.size() == 2)
      {
      std::string fullCommand = command;
      fullCommand += " update ";
      fullCommand += cvsArgs[1];
      output = "";
      retVal = 0;
      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Run CVS: "
        << fullCommand.c_str() << std::endl);
      res = cmSystemTools::RunSingleCommand(fullCommand.c_str(), &output,
        &retVal, cvsArgs[0].c_str(),
        m_HandlerVerbose, 0 /*m_TimeOut*/);
      if (!res || retVal != 0)
        {
        cmSystemTools::Error("Unable to perform extra cvs updates:\n",
          output.c_str());
        return 0;
        }
      }
    }
  return 0;
}


//----------------------------------------------------------------------
// run a single dashboard entry
int cmCTestScriptHandler::RunConfigurationDashboard()
{
  // local variables
  std::string command;
  std::string output;
  int retVal;
  bool res;

  // make sure the src directory is there, if it isn't then we might be able
  // to check it out from cvs
  retVal = this->CheckOutSourceDir();
  if (retVal)
    {
    return retVal;
    }

  // backup the dirs if requested
  retVal = this->BackupDirectories();
  if (retVal)
    {
    return retVal;
    }

  // clear the binary directory?
  if (m_EmptyBinDir)
    {
    if ( !cmCTestScriptHandler::EmptyBinaryDirectory(m_BinaryDir.c_str()) )
      {
      cmCTestLog(m_CTest, ERROR_MESSAGE,
        "Problem removing the binary directory" << std::endl);
      }
    }

  // make sure the binary directory exists if it isn't the srcdir
  if (!cmSystemTools::FileExists(m_BinaryDir.c_str()) &&
      m_SourceDir != m_BinaryDir)
    {
    if (!cmSystemTools::MakeDirectory(m_BinaryDir.c_str()))
      {
      cmSystemTools::Error("Unable to create the binary directory:\n",
                           m_BinaryDir.c_str());
      this->RestoreBackupDirectories();
      return 7;
      }
    }

  // if the binary directory and the source directory are the same,
  // and we are starting with an empty binary directory, then that means
  // we must check out the source tree
  if (m_EmptyBinDir && m_SourceDir == m_BinaryDir)
    {
    // make sure we have the required info
    if (m_CVSCheckOut.empty())
      {
      cmSystemTools::Error("You have specified the source and binary "
        "directories to be the same (an in source build). You have also "
        "specified that the binary directory is to be erased. This means "
        "that the source will have to be checked out from CVS. But you have "
        "not specified CTEST_CVS_CHECKOUT");
      return 8;
      }

    // we must now checkout the src dir
    retVal = this->CheckOutSourceDir();
    if (retVal)
      {
      this->RestoreBackupDirectories();
      return retVal;
      }
    }

  // backup the dirs if requested
  retVal = this->PerformExtraUpdates();
  if (retVal)
    {
    return retVal;
    }

  // put the initial cache into the bin dir
  if (!m_InitCache.empty())
    {
    std::string cacheFile = m_BinaryDir;
    cacheFile += "/CMakeCache.txt";
    cmGeneratedFileStream fout(cacheFile.c_str());
    if(!fout)
      {
      this->RestoreBackupDirectories();
      return 9;
      }

    fout.write(m_InitCache.c_str(), m_InitCache.size());

    // Make sure the operating system has finished writing the file
    // before closing it.  This will ensure the file is finished before
    // the check below.
    fout.flush();
    fout.close();
    }

  // do an initial cmake to setup the DartConfig file
  int cmakeFailed = 0;
  std::string cmakeFailedOuput;
  if (!m_CMakeCmd.empty())
    {
    command = m_CMakeCmd;
    command += " \"";
    command += m_SourceDir;
    output = "";
    command += "\"";
    retVal = 0;
    cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Run cmake command: "
      << command.c_str() << std::endl);
    res = cmSystemTools::RunSingleCommand(command.c_str(), &output,
      &retVal, m_BinaryDir.c_str(),
      m_HandlerVerbose, 0 /*m_TimeOut*/);

    if ( !m_CMOutFile.empty() )
      {
      std::string cmakeOutputFile = m_CMOutFile;
      if ( !cmSystemTools::FileIsFullPath(cmakeOutputFile.c_str()) )
        {
        cmakeOutputFile = m_BinaryDir + "/" + cmakeOutputFile;
        }

      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT,
        "Write CMake output to file: " << cmakeOutputFile.c_str()
        << std::endl);
      cmGeneratedFileStream fout(cmakeOutputFile.c_str());
      if ( fout )
        {
        fout << output.c_str();
        }
      else
        {
        cmCTestLog(m_CTest, ERROR_MESSAGE, "Cannot open CMake output file: "
          << cmakeOutputFile.c_str() << " for writing" << std::endl);
        }
      }
    if (!res || retVal != 0)
      {
      // even if this fails continue to the next step
      cmakeFailed = 1;
      cmakeFailedOuput = output;
      }
    }

  // run ctest, it may be more than one command in here
  std::vector<std::string> ctestCommands;
  cmSystemTools::ExpandListArgument(m_CTestCmd,ctestCommands);
  // for each variable/argument do a putenv
  for (unsigned i = 0; i < ctestCommands.size(); ++i)
    {
    command = ctestCommands[i];
    output = "";
    retVal = 0;
    cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Run ctest command: "
      << command.c_str() << std::endl);
    res = cmSystemTools::RunSingleCommand(command.c_str(), &output,
                                          &retVal, m_BinaryDir.c_str(),
                                          m_HandlerVerbose, 0 /*m_TimeOut*/);

    // did something critical fail in ctest
    if (!res || cmakeFailed ||
        retVal & cmCTest::BUILD_ERRORS)
      {
      this->RestoreBackupDirectories();
      if (cmakeFailed)
        {
        cmCTestLog(m_CTest, ERROR_MESSAGE, "Unable to run cmake:" << std::endl
          << cmakeFailedOuput.c_str() << std::endl);
        return 10;
        }
      cmCTestLog(m_CTest, ERROR_MESSAGE, "Unable to run ctest:" << std::endl
        << output.c_str() << std::endl);
      if (!res)
        {
        return 11;
        }
      return retVal * 100;
      }
    }

  // if all was succesful, delete the backup dirs to free up disk space
  if (m_Backup)
    {
    cmSystemTools::RemoveADirectory(m_BackupSourceDir.c_str());
    cmSystemTools::RemoveADirectory(m_BackupBinaryDir.c_str());
    }

  return 0;
}


//-------------------------------------------------------------------------
void cmCTestScriptHandler::RestoreBackupDirectories()
{
  // if we backed up the dirs and the build failed, then restore
  // the backed up dirs
  if (m_Backup)
    {
    // if for some reason those directories exist then first delete them
    if (cmSystemTools::FileExists(m_SourceDir.c_str()))
      {
      cmSystemTools::RemoveADirectory(m_SourceDir.c_str());
      }
    if (cmSystemTools::FileExists(m_BinaryDir.c_str()))
      {
      cmSystemTools::RemoveADirectory(m_BinaryDir.c_str());
      }
    // rename the src and binary directories
    rename(m_BackupSourceDir.c_str(), m_SourceDir.c_str());
    rename(m_BackupBinaryDir.c_str(), m_BinaryDir.c_str());
    }
}

bool cmCTestScriptHandler::RunScript(cmCTest* ctest, const char *sname)
{
  cmCTestScriptHandler* sh = new cmCTestScriptHandler();
  sh->SetCTestInstance(ctest);
  sh->AddConfigurationScript(sname);
  sh->ProcessHandler();
  delete sh;
  return true;
}

bool cmCTestScriptHandler::EmptyBinaryDirectory(const char *sname)
{
  // try to avoid deleting root
  if (!sname || strlen(sname) < 2)
    {
    return false;
    }

  // try to avoid deleting directories that we shouldn't
  std::string check = sname;
  check += "/CMakeCache.txt";
  if(cmSystemTools::FileExists(check.c_str()) &&
     !cmSystemTools::RemoveADirectory(sname))
    {
    return false;
    }
  return true;
}
