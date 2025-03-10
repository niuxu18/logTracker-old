/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cm_curl.h"

#include "cmCTest.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include <cmsys/Directory.hxx>
#include <cmsys/SystemInformation.hxx>
#include "cmDynamicLoader.h"
#include "cmGeneratedFileStream.h"
#include "cmXMLSafe.h"
#include "cmVersionMacros.h"
#include "cmCTestCommand.h"

#include "cmCTestBuildHandler.h"
#include "cmCTestBuildAndTestHandler.h"
#include "cmCTestConfigureHandler.h"
#include "cmCTestCoverageHandler.h"
#include "cmCTestMemCheckHandler.h"
#include "cmCTestScriptHandler.h"
#include "cmCTestTestHandler.h"
#include "cmCTestUpdateHandler.h"
#include "cmCTestSubmitHandler.h"

#include "cmVersion.h"

#include <cmsys/RegularExpression.hxx>
#include <cmsys/Process.h>
#include <cmsys/Glob.hxx>

#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <ctype.h>

#include <memory> // auto_ptr

#if defined(__BEOS__) && !defined(__HAIKU__)
#include <be/kernel/OS.h>   /* disable_debugger() API. */
#endif

#if defined(__HAIKU__)
#include <os/kernel/OS.h>   /* disable_debugger() API. */
#endif


#define DEBUGOUT std::cout << __LINE__ << " "; std::cout
#define DEBUGERR std::cerr << __LINE__ << " "; std::cerr

//----------------------------------------------------------------------
struct tm* cmCTest::GetNightlyTime(std::string str,
                                   bool tomorrowtag)
{
  struct tm* lctime;
  time_t tctime = time(0);
  lctime = gmtime(&tctime);
  char buf[1024];
  // add todays year day and month to the time in str because
  // curl_getdate no longer assumes the day is today
  sprintf(buf, "%d%02d%02d %s", 
          lctime->tm_year+1900, 
          lctime->tm_mon +1,
          lctime->tm_mday,
          str.c_str());
  cmCTestLog(this, OUTPUT, "Determine Nightly Start Time" << std::endl
    << "   Specified time: " << str.c_str() << std::endl);
  //Convert the nightly start time to seconds. Since we are
  //providing only a time and a timezone, the current date of
  //the local machine is assumed. Consequently, nightlySeconds
  //is the time at which the nightly dashboard was opened or
  //will be opened on the date of the current client machine.
  //As such, this time may be in the past or in the future.
  time_t ntime = curl_getdate(buf, &tctime);
  cmCTestLog(this, DEBUG, "   Get curl time: " << ntime << std::endl);
  tctime = time(0);
  cmCTestLog(this, DEBUG, "   Get the current time: " << tctime << std::endl);

  const int dayLength = 24 * 60 * 60;
  cmCTestLog(this, DEBUG, "Seconds: " << tctime << std::endl);
  while ( ntime > tctime )
    {
    // If nightlySeconds is in the past, this is the current
    // open dashboard, then return nightlySeconds.  If
    // nightlySeconds is in the future, this is the next
    // dashboard to be opened, so subtract 24 hours to get the
    // time of the current open dashboard
    ntime -= dayLength;
    cmCTestLog(this, DEBUG, "Pick yesterday" << std::endl);
    cmCTestLog(this, DEBUG, "   Future time, subtract day: " << ntime
      << std::endl);
    }
  while ( tctime > (ntime + dayLength) )
    {
    ntime += dayLength;
    cmCTestLog(this, DEBUG, "   Past time, add day: " << ntime << std::endl);
    }
  cmCTestLog(this, DEBUG, "nightlySeconds: " << ntime << std::endl);
  cmCTestLog(this, DEBUG, "   Current time: " << tctime
    << " Nightly time: " << ntime << std::endl);
  if ( tomorrowtag )
    {
    cmCTestLog(this, OUTPUT, "   Use future tag, Add a day" << std::endl);
    ntime += dayLength;
    }
  lctime = gmtime(&ntime);
  return lctime;
}

//----------------------------------------------------------------------
std::string cmCTest::CleanString(const std::string& str)
{
  std::string::size_type spos = str.find_first_not_of(" \n\t\r\f\v");
  std::string::size_type epos = str.find_last_not_of(" \n\t\r\f\v");
  if ( spos == str.npos )
    {
    return std::string();
    }
  if ( epos != str.npos )
    {
    epos = epos - spos + 1;
    }
  return str.substr(spos, epos);
}

//----------------------------------------------------------------------
std::string cmCTest::CurrentTime()
{
  time_t currenttime = time(0);
  struct tm* t = localtime(&currenttime);
  //return ::CleanString(ctime(&currenttime));
  char current_time[1024];
  if ( this->ShortDateFormat )
    {
    strftime(current_time, 1000, "%b %d %H:%M %Z", t);
    }
  else
    {
    strftime(current_time, 1000, "%a %b %d %H:%M:%S %Z %Y", t);
    }
  cmCTestLog(this, DEBUG, "   Current_Time: " << current_time << std::endl);
  return cmXMLSafe(cmCTest::CleanString(current_time)).str();
}

//----------------------------------------------------------------------
std::string cmCTest::MakeURLSafe(const std::string& str)
{
  cmOStringStream ost;
  char buffer[10];
  for ( std::string::size_type pos = 0; pos < str.size(); pos ++ )
    {
    unsigned char ch = str[pos];
    if ( ( ch > 126 || ch < 32 ||
           ch == '&' ||
           ch == '%' ||
           ch == '+' ||
           ch == '=' ||
           ch == '@'
          ) && ch != 9 )
      {
      sprintf(buffer, "%02x;", (unsigned int)ch);
      ost << buffer;
      }
    else
      {
      ost << ch;
      }
    }
  return ost.str();
}

//----------------------------------------------------------------------------
std::string cmCTest::DecodeURL(const std::string& in)
{
  std::string out;
  for(const char* c = in.c_str(); *c; ++c)
    {
    if(*c == '%' && isxdigit(*(c+1)) && isxdigit(*(c+2)))
      {
      char buf[3] = {*(c+1), *(c+2), 0};
      out.append(1, char(strtoul(buf, 0, 16)));
      c += 2;
      }
    else
      {
      out.append(1, *c);
      }
    }
  return out;
}

//----------------------------------------------------------------------
cmCTest::cmCTest()
{
  this->LabelSummary           = true;
  this->ParallelLevel          = 1;
  this->SubmitIndex            = 0;
  this->Failover               = false;
  this->BatchJobs              = false;
  this->ForceNewCTestProcess   = false;
  this->TomorrowTag            = false;
  this->Verbose                = false;
  
  this->Debug                  = false;
  this->ShowLineNumbers        = false;
  this->Quiet                  = false;
  this->ExtraVerbose           = false;
  this->ProduceXML             = false;
  this->ShowOnly               = false;
  this->RunConfigurationScript = false;
  this->TestModel              = cmCTest::EXPERIMENTAL;
  this->MaxTestNameWidth       = 30;
  this->InteractiveDebugMode   = true;
  this->TimeOut                = 0;
  this->CompressXMLFiles       = false;
  this->CTestConfigFile        = "";
  this->OutputLogFile          = 0;
  this->OutputLogFileLastTag   = -1;
  this->SuppressUpdatingCTestConfiguration = false;
  this->DartVersion            = 1;
  this->OutputTestOutputOnTestFailure = false;
  if(cmSystemTools::GetEnv("CTEST_OUTPUT_ON_FAILURE"))
    {
    this->OutputTestOutputOnTestFailure = true;
    }
  this->InitStreams();

  this->Parts[PartStart].SetName("Start");
  this->Parts[PartUpdate].SetName("Update");
  this->Parts[PartConfigure].SetName("Configure");
  this->Parts[PartBuild].SetName("Build");
  this->Parts[PartTest].SetName("Test");
  this->Parts[PartCoverage].SetName("Coverage");
  this->Parts[PartMemCheck].SetName("MemCheck");
  this->Parts[PartSubmit].SetName("Submit");
  this->Parts[PartNotes].SetName("Notes");
  this->Parts[PartExtraFiles].SetName("ExtraFiles");

  // Fill the part name-to-id map.
  for(Part p = PartStart; p != PartCount; p = Part(p+1))
    {
    this->PartMap[cmSystemTools::LowerCase(this->Parts[p].GetName())] = p;
    }

  this->ShortDateFormat        = true;

  this->TestingHandlers["build"]     = new cmCTestBuildHandler;
  this->TestingHandlers["buildtest"] = new cmCTestBuildAndTestHandler;
  this->TestingHandlers["coverage"]  = new cmCTestCoverageHandler;
  this->TestingHandlers["script"]    = new cmCTestScriptHandler;
  this->TestingHandlers["test"]      = new cmCTestTestHandler;
  this->TestingHandlers["update"]    = new cmCTestUpdateHandler;
  this->TestingHandlers["configure"] = new cmCTestConfigureHandler;
  this->TestingHandlers["memcheck"]  = new cmCTestMemCheckHandler;
  this->TestingHandlers["submit"]    = new cmCTestSubmitHandler;

  cmCTest::t_TestingHandlers::iterator it;
  for ( it = this->TestingHandlers.begin();
    it != this->TestingHandlers.end(); ++ it )
    {
    it->second->SetCTestInstance(this);
    }

  // Make sure we can capture the build tool output.
  cmSystemTools::EnableVSConsoleOutput();
}

//----------------------------------------------------------------------
cmCTest::~cmCTest()
{
  cmCTest::t_TestingHandlers::iterator it;
  for ( it = this->TestingHandlers.begin();
    it != this->TestingHandlers.end(); ++ it )
    {
    delete it->second;
    it->second = 0;
    }
  this->SetOutputLogFileName(0);
}

void cmCTest::SetParallelLevel(int level)
{
  this->ParallelLevel = level < 1 ? 1 : level;
}

//----------------------------------------------------------------------------
cmCTest::Part cmCTest::GetPartFromName(const char* name)
{
  // Look up by lower-case to make names case-insensitive.
  std::string lower_name = cmSystemTools::LowerCase(name);
  PartMapType::const_iterator i = this->PartMap.find(lower_name);
  if(i != this->PartMap.end())
    {
    return i->second;
    }

  // The string does not name a valid part.
  return PartCount;
}

//----------------------------------------------------------------------
int cmCTest::Initialize(const char* binary_dir, bool new_tag,
  bool verbose_tag)
{
  cmCTestLog(this, DEBUG, "Here: " << __LINE__ << std::endl);
  if(!this->InteractiveDebugMode)
    {
    this->BlockTestErrorDiagnostics();
    }
  else
    {
    cmSystemTools::PutEnv("CTEST_INTERACTIVE_DEBUG_MODE=1");
    }

  this->BinaryDir = binary_dir;
  cmSystemTools::ConvertToUnixSlashes(this->BinaryDir);

  this->UpdateCTestConfiguration();

  cmCTestLog(this, DEBUG, "Here: " << __LINE__ << std::endl);
  if ( this->ProduceXML )
    {
    cmCTestLog(this, DEBUG, "Here: " << __LINE__ << std::endl);
    cmCTestLog(this, OUTPUT,
      "   Site: " << this->GetCTestConfiguration("Site") << std::endl
      << "   Build name: " << this->GetCTestConfiguration("BuildName")
      << std::endl);
    cmCTestLog(this, DEBUG, "Produce XML is on" << std::endl);
    if ( this->TestModel == cmCTest::NIGHTLY &&
         this->GetCTestConfiguration("NightlyStartTime").empty() )
      {
      cmCTestLog(this, WARNING,
                 "WARNING: No nightly start time found please set in"
                 " CTestConfig.cmake or DartConfig.cmake" << std::endl);
      cmCTestLog(this, DEBUG, "Here: " << __LINE__ << std::endl);
      return 0;
      }
    }

  cmake cm;
  cmGlobalGenerator gg;
  gg.SetCMakeInstance(&cm);
  std::auto_ptr<cmLocalGenerator> lg(gg.CreateLocalGenerator());
  cmMakefile *mf = lg->GetMakefile();
  if ( !this->ReadCustomConfigurationFileTree(this->BinaryDir.c_str(), mf) )
    {
    cmCTestLog(this, DEBUG, "Cannot find custom configuration file tree"
      << std::endl);
    return 0;
    }

  if ( this->ProduceXML )
    {
    std::string testingDir = this->BinaryDir + "/Testing";
    if ( cmSystemTools::FileExists(testingDir.c_str()) )
      {
      if ( !cmSystemTools::FileIsDirectory(testingDir.c_str()) )
        {
        cmCTestLog(this, ERROR_MESSAGE, "File " << testingDir
          << " is in the place of the testing directory" << std::endl);
        return 0;
        }
      }
    else
      {
      if ( !cmSystemTools::MakeDirectory(testingDir.c_str()) )
        {
        cmCTestLog(this, ERROR_MESSAGE, "Cannot create directory "
          << testingDir << std::endl);
        return 0;
        }
      }
    std::string tagfile = testingDir + "/TAG";
    std::ifstream tfin(tagfile.c_str());
    std::string tag;
    time_t tctime = time(0);
    if ( this->TomorrowTag )
      {
      tctime += ( 24 * 60 * 60 );
      }
    struct tm *lctime = gmtime(&tctime);
    if ( tfin && cmSystemTools::GetLineFromStream(tfin, tag) )
      {
      int year = 0;
      int mon = 0;
      int day = 0;
      int hour = 0;
      int min = 0;
      sscanf(tag.c_str(), "%04d%02d%02d-%02d%02d",
             &year, &mon, &day, &hour, &min);
      if ( year != lctime->tm_year + 1900 ||
           mon != lctime->tm_mon+1 ||
           day != lctime->tm_mday )
        {
        tag = "";
        }
      std::string tagmode;
      if ( cmSystemTools::GetLineFromStream(tfin, tagmode) )
        {
        if (tagmode.size() > 4 && !this->Parts[PartStart])
          {
          this->TestModel = cmCTest::GetTestModelFromString(tagmode.c_str());
          }
        }
      tfin.close();
      }
    if (tag.size() == 0 || new_tag || this->Parts[PartStart])
      {
      cmCTestLog(this, DEBUG, "TestModel: " << this->GetTestModelString()
        << std::endl);
      cmCTestLog(this, DEBUG, "TestModel: " << this->TestModel << std::endl);
      if ( this->TestModel == cmCTest::NIGHTLY )
        {
        lctime = this->GetNightlyTime(
          this->GetCTestConfiguration("NightlyStartTime"), this->TomorrowTag);
        }
      char datestring[100];
      sprintf(datestring, "%04d%02d%02d-%02d%02d",
              lctime->tm_year + 1900,
              lctime->tm_mon+1,
              lctime->tm_mday,
              lctime->tm_hour,
              lctime->tm_min);
      tag = datestring;
      std::ofstream ofs(tagfile.c_str());
      if ( ofs )
        {
        ofs << tag << std::endl;
        ofs << this->GetTestModelString() << std::endl;
        }
      ofs.close();
      if ( verbose_tag )
        {
        cmCTestLog(this, OUTPUT, "Create new tag: " << tag << " - "
          << this->GetTestModelString() << std::endl);
        }
      }
    this->CurrentTag = tag;
    }
  return 1;
}

//----------------------------------------------------------------------
bool cmCTest::InitializeFromCommand(cmCTestCommand* command, bool first)
{
  if ( !first && !this->CurrentTag.empty() )
    {
    return true;
    }

  std::string src_dir
    = this->GetCTestConfiguration("SourceDirectory").c_str();
  std::string bld_dir = this->GetCTestConfiguration("BuildDirectory").c_str();
  this->DartVersion = 1;
  for(Part p = PartStart; p != PartCount; p = Part(p+1))
    {
    this->Parts[p].SubmitFiles.clear();
    }

  cmMakefile* mf = command->GetMakefile();
  std::string fname = src_dir;
  fname += "/CTestConfig.cmake";
  cmSystemTools::ConvertToUnixSlashes(fname);
  if ( cmSystemTools::FileExists(fname.c_str()) )
    {
    cmCTestLog(this, OUTPUT, "   Reading ctest configuration file: "
      << fname.c_str() << std::endl);
    bool readit = mf->ReadListFile(mf->GetCurrentListFile(),
      fname.c_str() );
    if(!readit)
      {
      std::string m = "Could not find include file: ";
      m += fname;
      command->SetError(m.c_str());
      return false;
      }
    }
  else if ( !first )
    {
    cmCTestLog(this, WARNING, "Cannot locate CTest configuration: "
      << fname.c_str() << std::endl);
    }
  else
    {
    cmCTestLog(this, HANDLER_OUTPUT, "   Cannot locate CTest configuration: "
      << fname.c_str() << std::endl
      << "   Delay the initialization of CTest" << std::endl);
    }

  this->SetCTestConfigurationFromCMakeVariable(mf, "NightlyStartTime",
    "CTEST_NIGHTLY_START_TIME");
  this->SetCTestConfigurationFromCMakeVariable(mf, "Site", "CTEST_SITE");
  this->SetCTestConfigurationFromCMakeVariable(mf, "BuildName",
    "CTEST_BUILD_NAME");
  const char* dartVersion = mf->GetDefinition("CTEST_DART_SERVER_VERSION");
  if ( dartVersion )
    {
    this->DartVersion = atoi(dartVersion);
    if ( this->DartVersion < 0 )
      {
      cmCTestLog(this, ERROR_MESSAGE, "Invalid Dart server version: "
        << dartVersion << ". Please specify the version number."
        << std::endl);
      return false;
      }
    }

  if ( !this->Initialize(bld_dir.c_str(), true, false) )
    {
    if ( this->GetCTestConfiguration("NightlyStartTime").empty() && first)
      {
      return true;
      }
    return false;
    }
  cmCTestLog(this, OUTPUT, "   Use " << this->GetTestModelString()
    << " tag: " << this->GetCurrentTag() << std::endl);
  return true;
}


//----------------------------------------------------------------------
bool cmCTest::UpdateCTestConfiguration()
{
  if ( this->SuppressUpdatingCTestConfiguration )
    {
    return true;
    }
  std::string fileName = this->CTestConfigFile;
  if ( fileName.empty() )
    {
    fileName = this->BinaryDir + "/CTestConfiguration.ini";
    if ( !cmSystemTools::FileExists(fileName.c_str()) )
      {
      fileName = this->BinaryDir + "/DartConfiguration.tcl";
      }
    }
  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, "UpdateCTestConfiguration  from :"
             << fileName.c_str() << "\n");
  if ( !cmSystemTools::FileExists(fileName.c_str()) )
    {
    // No need to exit if we are not producing XML
    if ( this->ProduceXML )
      {
      cmCTestLog(this, ERROR_MESSAGE, "Cannot find file: " << fileName.c_str()
        << std::endl);
      return false;
      }
    }
  else
    {
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, "Parse Config file:" 
               << fileName.c_str() << "\n");
    // parse the dart test file
    std::ifstream fin(fileName.c_str());

    if(!fin)
      {
      return false;
      }

    char buffer[1024];
    while ( fin )
      {
      buffer[0] = 0;
      fin.getline(buffer, 1023);
      buffer[1023] = 0;
      std::string line = cmCTest::CleanString(buffer);
      if(line.size() == 0)
        {
        continue;
        }
      while ( fin && (line[line.size()-1] == '\\') )
        {
        line = line.substr(0, line.size()-1);
        buffer[0] = 0;
        fin.getline(buffer, 1023);
        buffer[1023] = 0;
        line += cmCTest::CleanString(buffer);
        }
      if ( line[0] == '#' )
        {
        continue;
        }
      std::string::size_type cpos = line.find_first_of(":");
      if ( cpos == line.npos )
        {
        continue;
        }
      std::string key = line.substr(0, cpos);
      std::string value
        = cmCTest::CleanString(line.substr(cpos+1, line.npos));
      this->CTestConfiguration[key] = value;
      }
    fin.close();
    }
  if ( !this->GetCTestConfiguration("BuildDirectory").empty() )
    {
    this->BinaryDir = this->GetCTestConfiguration("BuildDirectory");
    cmSystemTools::ChangeDirectory(this->BinaryDir.c_str());
    }
  this->TimeOut = atoi(this->GetCTestConfiguration("TimeOut").c_str());
  if ( this->ProduceXML )
    {
    this->CompressXMLFiles = cmSystemTools::IsOn(
      this->GetCTestConfiguration("CompressSubmission").c_str());
    }
  return true;
}

//----------------------------------------------------------------------
void cmCTest::BlockTestErrorDiagnostics()
{
  cmSystemTools::PutEnv("DART_TEST_FROM_DART=1");
  cmSystemTools::PutEnv("DASHBOARD_TEST_FROM_CTEST=" CMake_VERSION);
#if defined(_WIN32)
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#elif defined(__BEOS__) || defined(__HAIKU__)
  disable_debugger(1);
#endif
}

//----------------------------------------------------------------------
void cmCTest::SetTestModel(int mode)
{
  this->InteractiveDebugMode = false;
  this->TestModel = mode;
}

//----------------------------------------------------------------------
bool cmCTest::SetTest(const char* ttype, bool report)
{
  if ( cmSystemTools::LowerCase(ttype) == "all" )
    {
    for(Part p = PartStart; p != PartCount; p = Part(p+1))
      {
      this->Parts[p].Enable();
      }
    return true;
    }
  Part p = this->GetPartFromName(ttype);
  if(p != PartCount)
    {
    this->Parts[p].Enable();
    return true;
    }
  else
    {
    if ( report )
      {
      cmCTestLog(this, ERROR_MESSAGE, "Don't know about test \"" << ttype
        << "\" yet..." << std::endl);
      }
    return false;
    }
}

//----------------------------------------------------------------------
void cmCTest::Finalize()
{
}

//----------------------------------------------------------------------
bool cmCTest::OpenOutputFile(const std::string& path,
                     const std::string& name, cmGeneratedFileStream& stream,
                     bool compress)
{
  std::string testingDir = this->BinaryDir + "/Testing";
  if ( path.size() > 0 )
    {
    testingDir += "/" + path;
    }
  if ( cmSystemTools::FileExists(testingDir.c_str()) )
    {
    if ( !cmSystemTools::FileIsDirectory(testingDir.c_str()) )
      {
      cmCTestLog(this, ERROR_MESSAGE, "File " << testingDir
                << " is in the place of the testing directory"
                << std::endl);
      return false;
      }
    }
  else
    {
    if ( !cmSystemTools::MakeDirectory(testingDir.c_str()) )
      {
      cmCTestLog(this, ERROR_MESSAGE, "Cannot create directory " << testingDir
                << std::endl);
      return false;
      }
    }
  std::string filename = testingDir + "/" + name;
  stream.Open(filename.c_str());
  if( !stream )
    {
    cmCTestLog(this, ERROR_MESSAGE, "Problem opening file: " << filename
      << std::endl);
    return false;
    }
  if ( compress )
    {
    if ( this->CompressXMLFiles )
      {
      stream.SetCompression(true);
      }
    }
  return true;
}

//----------------------------------------------------------------------
bool cmCTest::AddIfExists(Part part, const char* file)
{
  if ( this->CTestFileExists(file) )
    {
    this->AddSubmitFile(part, file);
    }
  else
    {
    std::string name = file;
    name += ".gz";
    if ( this->CTestFileExists(name.c_str()) )
      {
      this->AddSubmitFile(part, file);
      }
    else
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------
bool cmCTest::CTestFileExists(const std::string& filename)
{
  std::string testingDir = this->BinaryDir + "/Testing/" + 
    this->CurrentTag + "/" + filename;
  return cmSystemTools::FileExists(testingDir.c_str());
}

//----------------------------------------------------------------------
cmCTestGenericHandler* cmCTest::GetInitializedHandler(const char* handler)
{
  cmCTest::t_TestingHandlers::iterator it = 
    this->TestingHandlers.find(handler);
  if ( it == this->TestingHandlers.end() )
    {
    return 0;
    }
  it->second->Initialize();
  return it->second;
}

//----------------------------------------------------------------------
cmCTestGenericHandler* cmCTest::GetHandler(const char* handler)
{
  cmCTest::t_TestingHandlers::iterator it = 
    this->TestingHandlers.find(handler);
  if ( it == this->TestingHandlers.end() )
    {
    return 0;
    }
  return it->second;
}

//----------------------------------------------------------------------
int cmCTest::ExecuteHandler(const char* shandler)
{
  cmCTestGenericHandler* handler = this->GetHandler(shandler);
  if ( !handler )
    {
    return -1;
    }
  handler->Initialize();
  return handler->ProcessHandler();
}

//----------------------------------------------------------------------
int cmCTest::ProcessTests()
{
  int res = 0;
  bool notest = true;
  int update_count = 0;

  for(Part p = PartStart; notest && p != PartCount; p = Part(p+1))
    {
    notest = !this->Parts[p];
    }
  if (this->Parts[PartUpdate] &&
      (this->GetRemainingTimeAllowed() - 120 > 0))
    {
    cmCTestGenericHandler* uphandler = this->GetHandler("update");
    uphandler->SetPersistentOption("SourceDirectory",
      this->GetCTestConfiguration("SourceDirectory").c_str());
    update_count = uphandler->ProcessHandler();
    if ( update_count < 0 )
      {
      res |= cmCTest::UPDATE_ERRORS;
      }
    }
  if ( this->TestModel == cmCTest::CONTINUOUS && !update_count )
    {
    return 0;
    }
  if (this->Parts[PartConfigure] &&
      (this->GetRemainingTimeAllowed() - 120 > 0))
    {
    if (this->GetHandler("configure")->ProcessHandler() < 0)
      {
      res |= cmCTest::CONFIGURE_ERRORS;
      }
    }
  if (this->Parts[PartBuild] &&
      (this->GetRemainingTimeAllowed() - 120 > 0))
    {
    this->UpdateCTestConfiguration();
    if (this->GetHandler("build")->ProcessHandler() < 0)
      {
      res |= cmCTest::BUILD_ERRORS;
      }
    }
  if ((this->Parts[PartTest] || notest) &&
      (this->GetRemainingTimeAllowed() - 120 > 0))
    {
    this->UpdateCTestConfiguration();
    if (this->GetHandler("test")->ProcessHandler() < 0)
      {
      res |= cmCTest::TEST_ERRORS;
      }
    }
  if (this->Parts[PartCoverage] &&
      (this->GetRemainingTimeAllowed() - 120 > 0))
    {
    this->UpdateCTestConfiguration();
    if (this->GetHandler("coverage")->ProcessHandler() < 0)
      {
      res |= cmCTest::COVERAGE_ERRORS;
      }
    }
  if (this->Parts[PartMemCheck] &&
      (this->GetRemainingTimeAllowed() - 120 > 0))
    {
    this->UpdateCTestConfiguration();
    if (this->GetHandler("memcheck")->ProcessHandler() < 0)
      {
      res |= cmCTest::MEMORY_ERRORS;
      }
    }
  if ( !notest )
    {
    std::string notes_dir = this->BinaryDir + "/Testing/Notes";
    if ( cmSystemTools::FileIsDirectory(notes_dir.c_str()) )
      {
      cmsys::Directory d;
      d.Load(notes_dir.c_str());
      unsigned long kk;
      for ( kk = 0; kk < d.GetNumberOfFiles(); kk ++ )
        {
        const char* file = d.GetFile(kk);
        std::string fullname = notes_dir + "/" + file;
        if ( cmSystemTools::FileExists(fullname.c_str()) &&
          !cmSystemTools::FileIsDirectory(fullname.c_str()) )
          {
          if ( this->NotesFiles.size() > 0 )
            {
            this->NotesFiles += ";";
            }
          this->NotesFiles += fullname;
          this->Parts[PartNotes].Enable();
          }
        }
      }
    }
  if (this->Parts[PartNotes])
    {
    this->UpdateCTestConfiguration();
    if ( this->NotesFiles.size() )
      {
      this->GenerateNotesFile(this->NotesFiles.c_str());
      }
    }
  if (this->Parts[PartSubmit])
    {
    this->UpdateCTestConfiguration();
    if (this->GetHandler("submit")->ProcessHandler() < 0)
      {
      res |= cmCTest::SUBMIT_ERRORS;
      }
    }
  if ( res != 0 )
    {
    cmCTestLog(this, ERROR_MESSAGE, "Errors while running CTest"
                 << std::endl);
    }
  return res;
}

//----------------------------------------------------------------------
std::string cmCTest::GetTestModelString()
{
  if ( !this->SpecificTrack.empty() )
    {
    return this->SpecificTrack;
    }
  switch ( this->TestModel )
    {
  case cmCTest::NIGHTLY:
    return "Nightly";
  case cmCTest::CONTINUOUS:
    return "Continuous";
    }
  return "Experimental";
}

//----------------------------------------------------------------------
int cmCTest::GetTestModelFromString(const char* str)
{
  if ( !str )
    {
    return cmCTest::EXPERIMENTAL;
    }
  std::string rstr = cmSystemTools::LowerCase(str);
  if ( strncmp(rstr.c_str(), "cont", 4) == 0 )
    {
    return cmCTest::CONTINUOUS;
    }
  if ( strncmp(rstr.c_str(), "nigh", 4) == 0 )
    {
    return cmCTest::NIGHTLY;
    }
  return cmCTest::EXPERIMENTAL;
}

//######################################################################
//######################################################################
//######################################################################
//######################################################################

//----------------------------------------------------------------------
int cmCTest::RunMakeCommand(const char* command, std::string* output,
  int* retVal, const char* dir, int timeout, std::ofstream& ofs)
{
  // First generate the command and arguments
  std::vector<cmStdString> args = cmSystemTools::ParseArguments(command);

  if(args.size() < 1)
    {
    return false;
    }

  std::vector<const char*> argv;
  for(std::vector<cmStdString>::const_iterator a = args.begin();
    a != args.end(); ++a)
    {
    argv.push_back(a->c_str());
    }
  argv.push_back(0);

  if ( output )
    {
    *output = "";
    }

  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, "Run command:");
  std::vector<const char*>::iterator ait;
  for ( ait = argv.begin(); ait != argv.end() && *ait; ++ ait )
    {
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, " \"" << *ait << "\"");
    }
  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, std::endl);

  // Now create process object
  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, &*argv.begin());
  cmsysProcess_SetWorkingDirectory(cp, dir);
  cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
  cmsysProcess_SetTimeout(cp, timeout);
  cmsysProcess_Execute(cp);

  // Initialize tick's
  std::string::size_type tick = 0;
  std::string::size_type tick_len = 1024;
  std::string::size_type tick_line_len = 50;

  char* data;
  int length;
  cmCTestLog(this, HANDLER_OUTPUT,
    "   Each . represents " << tick_len << " bytes of output" << std::endl
    << "    " << std::flush);
  while(cmsysProcess_WaitForData(cp, &data, &length, 0))
    {
    if ( output )
      {
      for(int cc =0; cc < length; ++cc)
        {
        if(data[cc] == 0)
          {
          data[cc] = '\n';
          }
        }

      output->append(data, length);
      while ( output->size() > (tick * tick_len) )
        {
        tick ++;
        cmCTestLog(this, HANDLER_OUTPUT, "." << std::flush);
        if ( tick % tick_line_len == 0 && tick > 0 )
          {
          cmCTestLog(this, HANDLER_OUTPUT, "  Size: "
            << int((output->size() / 1024.0) + 1) << "K" << std::endl
            << "    " << std::flush);
          }
        }
      }
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, cmCTestLogWrite(data, length));
    if ( ofs )
      {
      ofs << cmCTestLogWrite(data, length);
      }
    }
  cmCTestLog(this, OUTPUT, " Size of output: "
    << int(output->size() / 1024.0) << "K" << std::endl);

  cmsysProcess_WaitForExit(cp, 0);

  int result = cmsysProcess_GetState(cp);

  if(result == cmsysProcess_State_Exited)
    {
    *retVal = cmsysProcess_GetExitValue(cp);
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, "Command exited with the value: "
      << *retVal << std::endl);
    }
  else if(result == cmsysProcess_State_Exception)
    {
    *retVal = cmsysProcess_GetExitException(cp);
    cmCTestLog(this, WARNING, "There was an exception: " << *retVal
      << std::endl);
    }
  else if(result == cmsysProcess_State_Expired)
    {
    cmCTestLog(this, WARNING, "There was a timeout" << std::endl);
    }
  else if(result == cmsysProcess_State_Error)
    {
    *output += "\n*** ERROR executing: ";
    *output += cmsysProcess_GetErrorString(cp);
    *output += "\n***The build process failed.";
    cmCTestLog(this, ERROR_MESSAGE, "There was an error: "
      << cmsysProcess_GetErrorString(cp) << std::endl);
    }

  cmsysProcess_Delete(cp);

  return result;
}

//######################################################################
//######################################################################
//######################################################################
//######################################################################

//----------------------------------------------------------------------
int cmCTest::RunTest(std::vector<const char*> argv,
                     std::string* output, int *retVal,
                     std::ostream* log, double testTimeOut,
                     std::vector<std::string>* environment)
{
  std::vector<std::string> origEnv;
  bool modifyEnv = (environment && environment->size()>0);

  // determine how much time we have
  double timeout = this->GetRemainingTimeAllowed() - 120;
  if (this->TimeOut > 0 && this->TimeOut < timeout)
    {
    timeout = this->TimeOut;
    }
  if (testTimeOut > 0
      && testTimeOut < this->GetRemainingTimeAllowed())
    {
    timeout = testTimeOut;
    }

  // always have at least 1 second if we got to here
  if (timeout <= 0)
    {
    timeout = 1;
    }
  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT,
             "Test timeout computed to be: " << timeout << "\n");
  if(cmSystemTools::SameFile(argv[0], this->CTestSelf.c_str()) &&
     !this->ForceNewCTestProcess)
    {
    cmCTest inst;
    inst.ConfigType = this->ConfigType;
    inst.TimeOut = timeout;

    // Capture output of the child ctest.
    cmOStringStream oss;
    inst.SetStreams(&oss, &oss);

    std::vector<std::string> args;
    for(unsigned int i =0; i < argv.size(); ++i)
      {
      if(argv[i])
        {
        // make sure we pass the timeout in for any build and test 
        // invocations. Since --build-generator is required this is a 
        // good place to check for it, and to add the arguments in
        if (strcmp(argv[i],"--build-generator") == 0 && timeout > 0)
          {
          args.push_back("--test-timeout");
          cmOStringStream msg;
          msg << timeout;
          args.push_back(msg.str());
          }
        args.push_back(argv[i]);
        }
      }
    if ( log )
      {
      *log << "* Run internal CTest" << std::endl;
      }
    std::string oldpath = cmSystemTools::GetCurrentWorkingDirectory();

    if (modifyEnv)
      {
      origEnv = cmSystemTools::AppendEnv(environment);
      }

    *retVal = inst.Run(args, output);
    *output += oss.str();
    if ( log )
      {
      *log << output->c_str();
      }
    cmSystemTools::ChangeDirectory(oldpath.c_str());

    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT,
      "Internal cmCTest object used to run test." << std::endl
      <<  *output << std::endl);

    if (modifyEnv)
      {
      cmSystemTools::RestoreEnv(origEnv);
      }

    return cmsysProcess_State_Exited;
    }
  std::vector<char> tempOutput;
  if ( output )
    {
    *output = "";
    }

  if (modifyEnv)
    {
    origEnv = cmSystemTools::AppendEnv(environment);
    }

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, &*argv.begin());
  cmCTestLog(this, DEBUG, "Command is: " << argv[0] << std::endl);
  if(cmSystemTools::GetRunCommandHideConsole())
    {
    cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
    }

  cmsysProcess_SetTimeout(cp, timeout);
  cmsysProcess_Execute(cp);

  char* data;
  int length;
  while(cmsysProcess_WaitForData(cp, &data, &length, 0))
    {
    if ( output )
      {
      tempOutput.insert(tempOutput.end(), data, data+length);
      }
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, cmCTestLogWrite(data, length));
    if ( log )
      {
      log->write(data, length);
      }
    }

  cmsysProcess_WaitForExit(cp, 0);
  if(output && tempOutput.begin() != tempOutput.end())
    {
    output->append(&*tempOutput.begin(), tempOutput.size());
    }
  cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, "-- Process completed"
    << std::endl);

  int result = cmsysProcess_GetState(cp);

  if(result == cmsysProcess_State_Exited)
    {
    *retVal = cmsysProcess_GetExitValue(cp);
    if(*retVal != 0 && this->OutputTestOutputOnTestFailure)
      {
        OutputTestErrors(tempOutput);
      }
    }
  else if(result == cmsysProcess_State_Exception)
    {
    if(this->OutputTestOutputOnTestFailure)
      {
        OutputTestErrors(tempOutput);
      }
    *retVal = cmsysProcess_GetExitException(cp);
    std::string outerr = "\n*** Exception executing: ";
    outerr += cmsysProcess_GetExceptionString(cp);
    *output += outerr;
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, outerr.c_str() << std::endl
      << std::flush);
    }
  else if(result == cmsysProcess_State_Error)
    {
    std::string outerr = "\n*** ERROR executing: ";
    outerr += cmsysProcess_GetErrorString(cp);
    *output += outerr;
    cmCTestLog(this, HANDLER_VERBOSE_OUTPUT, outerr.c_str() << std::endl
      << std::flush);
    }
  cmsysProcess_Delete(cp);

  if (modifyEnv)
    {
    cmSystemTools::RestoreEnv(origEnv);
    }

  return result;
}

//----------------------------------------------------------------------
void cmCTest::StartXML(std::ostream& ostr, bool append)
{
  if(this->CurrentTag.empty())
    {
    cmCTestLog(this, ERROR_MESSAGE,
               "Current Tag empty, this may mean"
               " NightlStartTime was not set correctly." << std::endl);
    cmSystemTools::SetFatalErrorOccured();
    }
  // find out about the system
  cmsys::SystemInformation info;
  info.RunCPUCheck();
  info.RunOSCheck();
  info.RunMemoryCheck();
  ostr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       << "<Site BuildName=\"" << this->GetCTestConfiguration("BuildName")
       << "\"\n\tBuildStamp=\"" << this->CurrentTag << "-"
       << this->GetTestModelString() << "\"\n\tName=\""
       << this->GetCTestConfiguration("Site") << "\"\n\tGenerator=\"ctest-"
       << cmVersion::GetCMakeVersion()  << "\"\n"
       << (append? "\tAppend=\"true\"\n":"")
       << "\tCompilerName=\"" << this->GetCTestConfiguration("Compiler") 
       << "\"\n"
#ifdef _COMPILER_VERSION
       << "\tCompilerVersion=\"_COMPILER_VERSION\"\n"
#endif
       << "\tOSName=\"" << info.GetOSName() << "\"\n"
       << "\tHostname=\"" << info.GetHostname() << "\"\n"
       << "\tOSRelease=\"" << info.GetOSRelease() << "\"\n"
       << "\tOSVersion=\"" << info.GetOSVersion() << "\"\n"
       << "\tOSPlatform=\"" << info.GetOSPlatform() << "\"\n"
       << "\tIs64Bits=\"" << info.Is64Bits() << "\"\n"
       << "\tVendorString=\"" << info.GetVendorString() << "\"\n"
       << "\tVendorID=\"" << info.GetVendorID() << "\"\n"
       << "\tFamilyID=\"" << info.GetFamilyID() << "\"\n"
       << "\tModelID=\"" << info.GetModelID() << "\"\n"
       << "\tProcessorCacheSize=\"" << info.GetProcessorCacheSize() << "\"\n"
       << "\tNumberOfLogicalCPU=\"" << info.GetNumberOfLogicalCPU() << "\"\n"
       << "\tNumberOfPhysicalCPU=\""<< info.GetNumberOfPhysicalCPU() << "\"\n"
       << "\tTotalVirtualMemory=\"" << info.GetTotalVirtualMemory() << "\"\n"
       << "\tTotalPhysicalMemory=\""<< info.GetTotalPhysicalMemory() << "\"\n"
       << "\tLogicalProcessorsPerPhysical=\"" 
       << info.GetLogicalProcessorsPerPhysical() << "\"\n"
       << "\tProcessorClockFrequency=\"" 
       << info.GetProcessorClockFrequency() << "\"\n" 
       << ">" << std::endl;
  this->AddSiteProperties(ostr); 
}

//----------------------------------------------------------------------
void cmCTest::AddSiteProperties(std::ostream& ostr)
{
  cmCTestScriptHandler* ch = 
    static_cast<cmCTestScriptHandler*>(this->GetHandler("script"));
  cmake* cm =  ch->GetCMake();
  // if no CMake then this is the old style script and props like
  // this will not work anyway.
  if(!cm)
    {
    return;
    }
  // This code should go when cdash is changed to use labels only
  const char* subproject = cm->GetProperty("SubProject", cmProperty::GLOBAL);
  if(subproject)
    { 
    ostr << "<Subproject name=\"" << subproject << "\">\n";
    const char* labels = 
      ch->GetCMake()->GetProperty("SubProjectLabels", cmProperty::GLOBAL);
    if(labels)
      {
      ostr << "  <Labels>\n";
      std::string l = labels;
      std::vector<std::string> args;
      cmSystemTools::ExpandListArgument(l, args);
      for(std::vector<std::string>::iterator i = args.begin();
          i != args.end(); ++i)
        {
        ostr << "    <Label>" << i->c_str() << "</Label>\n";
        }
      ostr << "  </Labels>\n";
      }
    ostr << "</Subproject>\n";
    }
  
  // This code should stay when cdash only does label based sub-projects
  const char* label = cm->GetProperty("Label", cmProperty::GLOBAL);
  if(label)
    { 
    ostr << "<Labels>\n";
    ostr << "  <Label>" << label << "</Label>\n";
    ostr << "</Labels>\n";
    }
}


//----------------------------------------------------------------------
void cmCTest::EndXML(std::ostream& ostr)
{
  ostr << "</Site>" << std::endl;
}

//----------------------------------------------------------------------
int cmCTest::GenerateCTestNotesOutput(std::ostream& os,
  const cmCTest::VectorOfStrings& files)
{
  cmCTest::VectorOfStrings::const_iterator it;
  os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
     << "<?xml-stylesheet type=\"text/xsl\" "
    "href=\"Dart/Source/Server/XSL/Build.xsl "
    "<file:///Dart/Source/Server/XSL/Build.xsl> \"?>\n"
     << "<Site BuildName=\"" << this->GetCTestConfiguration("BuildName")
     << "\" BuildStamp=\""
     << this->CurrentTag << "-" << this->GetTestModelString() << "\" Name=\""
     << this->GetCTestConfiguration("Site") << "\" Generator=\"ctest"
     << cmVersion::GetCMakeVersion()
     << "\">\n";
  this->AddSiteProperties(os);
  os << "<Notes>" << std::endl;

  for ( it = files.begin(); it != files.end(); it ++ )
    {
    cmCTestLog(this, OUTPUT, "\tAdd file: " << it->c_str() << std::endl);
    std::string note_time = this->CurrentTime();
    os << "<Note Name=\"" << cmXMLSafe(*it) << "\">\n"
      << "<Time>" << cmSystemTools::GetTime() << "</Time>\n"
      << "<DateTime>" << note_time << "</DateTime>\n"
      << "<Text>" << std::endl;
    std::ifstream ifs(it->c_str());
    if ( ifs )
      {
      std::string line;
      while ( cmSystemTools::GetLineFromStream(ifs, line) )
        {
        os << cmXMLSafe(line) << std::endl;
        }
      ifs.close();
      }
    else
      {
      os << "Problem reading file: " << it->c_str() << std::endl;
      cmCTestLog(this, ERROR_MESSAGE, "Problem reading file: " << it->c_str()
        << " while creating notes" << std::endl);
      }
    os << "</Text>\n"
      << "</Note>" << std::endl;
    }
  os << "</Notes>\n"
    << "</Site>" << std::endl;
  return 1;
}

//----------------------------------------------------------------------
int cmCTest::GenerateNotesFile(const std::vector<cmStdString> &files)
{
  cmGeneratedFileStream ofs;
  if ( !this->OpenOutputFile(this->CurrentTag, "Notes.xml", ofs) )
    {
    cmCTestLog(this, ERROR_MESSAGE, "Cannot open notes file" << std::endl);
    return 1;
    }

  this->GenerateCTestNotesOutput(ofs, files);
  return 0;
}

//----------------------------------------------------------------------
int cmCTest::GenerateNotesFile(const char* cfiles)
{
  if ( !cfiles )
    {
    return 1;
    }

  std::vector<cmStdString> files;

  cmCTestLog(this, OUTPUT, "Create notes file" << std::endl);

  files = cmSystemTools::SplitString(cfiles, ';');
  if ( files.size() == 0 )
    {
    return 1;
    }

  return this->GenerateNotesFile(files);
}

//----------------------------------------------------------------------
bool cmCTest::SubmitExtraFiles(const std::vector<cmStdString> &files)
{
  std::vector<cmStdString>::const_iterator it;
  for ( it = files.begin();
    it != files.end();
    ++ it )
    {
    if ( !cmSystemTools::FileExists(it->c_str()) )
      {
      cmCTestLog(this, ERROR_MESSAGE, "Cannot find extra file: "
        << it->c_str() << " to submit."
        << std::endl;);
      return false;
      }
    this->AddSubmitFile(PartExtraFiles, it->c_str());
    }
  return true;
}

//----------------------------------------------------------------------
bool cmCTest::SubmitExtraFiles(const char* cfiles)
{
  if ( !cfiles )
    {
    return 1;
    }

  std::vector<cmStdString> files;

  cmCTestLog(this, OUTPUT, "Submit extra files" << std::endl);

  files = cmSystemTools::SplitString(cfiles, ';');
  if ( files.size() == 0 )
    {
    return 1;
    }

  return this->SubmitExtraFiles(files);
}


//-------------------------------------------------------
// for a -D argument convert the next argument into 
// the proper list of dashboard steps via SetTest
bool cmCTest::AddTestsForDashboardType(std::string &targ)
{
  if ( targ == "Experimental" )
    {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Start");
    this->SetTest("Configure");
    this->SetTest("Build");
    this->SetTest("Test");
    this->SetTest("Coverage");
    this->SetTest("Submit");
    }
  else if ( targ == "ExperimentalStart" )
    {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Start");
    }
  else if ( targ == "ExperimentalUpdate" )
    {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Update");
    }
  else if ( targ == "ExperimentalConfigure" )
    {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Configure");
    }
  else if ( targ == "ExperimentalBuild" )
    {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Build");
    }
  else if ( targ == "ExperimentalTest" )
    {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Test");
    }
  else if ( targ == "ExperimentalMemCheck"
            || targ == "ExperimentalPurify" )
    {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("MemCheck");
    }
  else if ( targ == "ExperimentalCoverage" )
    {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Coverage");
    }
  else if ( targ == "ExperimentalSubmit" )
    {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Submit");
    }
  else if ( targ == "Continuous" )
    {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Start");
    this->SetTest("Update");
    this->SetTest("Configure");
    this->SetTest("Build");
    this->SetTest("Test");
    this->SetTest("Coverage");
    this->SetTest("Submit");
    }
  else if ( targ == "ContinuousStart" )
    {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Start");
    }
  else if ( targ == "ContinuousUpdate" )
    {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Update");
    }
  else if ( targ == "ContinuousConfigure" )
    {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Configure");
    }
  else if ( targ == "ContinuousBuild" )
    {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Build");
    }
  else if ( targ == "ContinuousTest" )
    {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Test");
    }
  else if ( targ == "ContinuousMemCheck"
        || targ == "ContinuousPurify" )
    {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("MemCheck");
    }
  else if ( targ == "ContinuousCoverage" )
    {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Coverage");
    }
  else if ( targ == "ContinuousSubmit" )
    {
    this->SetTestModel(cmCTest::CONTINUOUS);
    this->SetTest("Submit");
    }
  else if ( targ == "Nightly" )
    {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Start");
    this->SetTest("Update");
    this->SetTest("Configure");
    this->SetTest("Build");
    this->SetTest("Test");
    this->SetTest("Coverage");
    this->SetTest("Submit");
    }
  else if ( targ == "NightlyStart" )
    {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Start");
    }
  else if ( targ == "NightlyUpdate" )
    {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Update");
    }
  else if ( targ == "NightlyConfigure" )
    {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Configure");
    }
  else if ( targ == "NightlyBuild" )
    {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Build");
    }
  else if ( targ == "NightlyTest" )
    {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Test");
    }
  else if ( targ == "NightlyMemCheck"
            || targ == "NightlyPurify" )
    {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("MemCheck");
    }
  else if ( targ == "NightlyCoverage" )
    {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Coverage");
    }
  else if ( targ == "NightlySubmit" )
    {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Submit");
    }
  else if ( targ == "MemoryCheck" )
    {
    this->SetTestModel(cmCTest::EXPERIMENTAL);
    this->SetTest("Start");
    this->SetTest("Configure");
    this->SetTest("Build");
    this->SetTest("MemCheck");
    this->SetTest("Coverage");
    this->SetTest("Submit");
    }
  else if ( targ == "NightlyMemoryCheck" )
    {
    this->SetTestModel(cmCTest::NIGHTLY);
    this->SetTest("Start");
    this->SetTest("Update");
    this->SetTest("Configure");
    this->SetTest("Build");
    this->SetTest("MemCheck");
    this->SetTest("Coverage");
    this->SetTest("Submit");
    }
  else
    {
    cmCTestLog(this, ERROR_MESSAGE,
               "CTest -D called with incorrect option: "
               << targ << std::endl);
    cmCTestLog(this, ERROR_MESSAGE, "Available options are:" << std::endl
               << "  " << "ctest" << " -D Continuous" << std::endl
               << "  " << "ctest"
               << " -D Continuous(Start|Update|Configure|Build)" << std::endl
               << "  " << "ctest"
               << " -D Continuous(Test|Coverage|MemCheck|Submit)"
               << std::endl
               << "  " << "ctest" << " -D Experimental" << std::endl
               << "  " << "ctest"
               << " -D Experimental(Start|Update|Configure|Build)"
               << std::endl
               << "  " << "ctest"
               << " -D Experimental(Test|Coverage|MemCheck|Submit)"
               << std::endl
               << "  " << "ctest" << " -D Nightly" << std::endl
               << "  " << "ctest"
               << " -D Nightly(Start|Update|Configure|Build)" << std::endl
               << "  " << "ctest"
               << " -D Nightly(Test|Coverage|MemCheck|Submit)" << std::endl
               << "  " << "ctest" << " -D NightlyMemoryCheck" << std::endl);
    return false;
    }
  return true;
}


//----------------------------------------------------------------------
bool cmCTest::CheckArgument(const std::string& arg, const char* varg1,
  const char* varg2)
{
  return (varg1 && arg == varg1) || (varg2 && arg == varg2);
}


//----------------------------------------------------------------------
// Processes one command line argument (and its arguments if any) 
// for many simple options and then returns
void cmCTest::HandleCommandLineArguments(size_t &i, 
                                         std::vector<std::string> &args)
{
  std::string arg = args[i];

  if(this->CheckArgument(arg, "-F"))
    {
    this->Failover = true;
    }
  if(this->CheckArgument(arg, "-j", "--parallel") && i < args.size() - 1)
    {
    i++;
    int plevel = atoi(args[i].c_str());
    this->SetParallelLevel(plevel);
    }
  else if(arg.find("-j") == 0)
    {
    int plevel = atoi(arg.substr(2).c_str());
    this->SetParallelLevel(plevel);
    }
  
  if(this->CheckArgument(arg, "-C", "--build-config") &&
     i < args.size() - 1)
    {
    i++;
    this->SetConfigType(args[i].c_str());
    }
  
  if(this->CheckArgument(arg, "--debug"))
    {
    this->Debug = true;
    this->ShowLineNumbers = true;
    }
  if(this->CheckArgument(arg, "--track") && i < args.size() - 1)
    {
    i++;
    this->SpecificTrack = args[i];
    }
  if(this->CheckArgument(arg, "--show-line-numbers"))
    {
    this->ShowLineNumbers = true;
    }
  if(this->CheckArgument(arg, "--no-label-summary"))
    {
    this->LabelSummary = false;
    }
  if(this->CheckArgument(arg, "-Q", "--quiet"))
    {
    this->Quiet = true;
    }
  if(this->CheckArgument(arg, "-V", "--verbose"))
    {
    this->Verbose = true;
    }
  if(this->CheckArgument(arg, "-B"))
    {
    this->BatchJobs = true;
    }
  if(this->CheckArgument(arg, "-VV", "--extra-verbose"))
    {
    this->ExtraVerbose = true;
    this->Verbose = true;
    }
  if(this->CheckArgument(arg, "--output-on-failure"))
    {
    this->OutputTestOutputOnTestFailure = true;
    }
  
  if(this->CheckArgument(arg, "-N", "--show-only"))
    {
    this->ShowOnly = true;
    }

  if(this->CheckArgument(arg, "-O", "--output-log") && i < args.size() - 1 )
    {
    i++;
    this->SetOutputLogFileName(args[i].c_str());
    }

  if(this->CheckArgument(arg, "--tomorrow-tag"))
    {
    this->TomorrowTag = true;
    }
  if(this->CheckArgument(arg, "--force-new-ctest-process"))
    {
    this->ForceNewCTestProcess = true;
    }
  if(this->CheckArgument(arg, "-W", "--max-width") && i < args.size() - 1)
    {
    i++;
    this->MaxTestNameWidth = atoi(args[i].c_str());
    }
  if(this->CheckArgument(arg, "--interactive-debug-mode") &&
     i < args.size() - 1 )
    {
    i++;
    this->InteractiveDebugMode = cmSystemTools::IsOn(args[i].c_str());
    }
  if(this->CheckArgument(arg, "--submit-index") && i < args.size() - 1 )
    {
    i++;
    this->SubmitIndex = atoi(args[i].c_str());
    if ( this->SubmitIndex < 0 )
      {
      this->SubmitIndex = 0;
      }
    }
  
  if(this->CheckArgument(arg, "--overwrite") && i < args.size() - 1)
    {
    i++;
    this->AddCTestConfigurationOverwrite(args[i].c_str());
    }
  if(this->CheckArgument(arg, "-A", "--add-notes") && i < args.size() - 1)
    {
    this->ProduceXML = true;
    this->SetTest("Notes");
    i++;
    this->SetNotesFiles(args[i].c_str());
    }

  // options that control what tests are run
  if(this->CheckArgument(arg, "-I", "--tests-information") &&
     i < args.size() - 1)
    {
    i++;
    this->GetHandler("test")->SetPersistentOption("TestsToRunInformation",
                                                  args[i].c_str());
    this->GetHandler("memcheck")->
      SetPersistentOption("TestsToRunInformation",args[i].c_str());
    }
  if(this->CheckArgument(arg, "-U", "--union"))
    {
    this->GetHandler("test")->SetPersistentOption("UseUnion", "true");
    this->GetHandler("memcheck")->SetPersistentOption("UseUnion", "true");
    }
  if(this->CheckArgument(arg, "-R", "--tests-regex") && i < args.size() - 1)
    {
    i++;
    this->GetHandler("test")->
      SetPersistentOption("IncludeRegularExpression", args[i].c_str());
    this->GetHandler("memcheck")->
      SetPersistentOption("IncludeRegularExpression", args[i].c_str());
    }
  if(this->CheckArgument(arg, "-L", "--label-regex") && i < args.size() - 1)
    {
    i++;
    this->GetHandler("test")->
      SetPersistentOption("LabelRegularExpression", args[i].c_str());
    this->GetHandler("memcheck")->
      SetPersistentOption("LabelRegularExpression", args[i].c_str());
    }
  if(this->CheckArgument(arg, "-LE", "--label-exclude") && i < args.size() - 1)
    {
    i++;
    this->GetHandler("test")->
      SetPersistentOption("ExcludeLabelRegularExpression", args[i].c_str());
    this->GetHandler("memcheck")->
      SetPersistentOption("ExcludeLabelRegularExpression", args[i].c_str());
    }
  
  if(this->CheckArgument(arg, "-E", "--exclude-regex") &&
     i < args.size() - 1)
    {
    i++;
    this->GetHandler("test")->
      SetPersistentOption("ExcludeRegularExpression", args[i].c_str());
    this->GetHandler("memcheck")->
      SetPersistentOption("ExcludeRegularExpression", args[i].c_str());
    }  
}

//----------------------------------------------------------------------
// handle the -S -SR and -SP arguments
void cmCTest::HandleScriptArguments(size_t &i, 
                                    std::vector<std::string> &args,
                                    bool &SRArgumentSpecified)
{
  std::string arg = args[i];
  if(this->CheckArgument(arg, "-SP", "--script-new-process") && 
     i < args.size() - 1 )
    {
    this->RunConfigurationScript = true;
    i++;
    cmCTestScriptHandler* ch
      = static_cast<cmCTestScriptHandler*>(this->GetHandler("script"));
    // -SR is an internal argument, -SP should be ignored when it is passed
    if (!SRArgumentSpecified)
      {
      ch->AddConfigurationScript(args[i].c_str(),false);
      }
    }
  
  if(this->CheckArgument(arg, "-SR", "--script-run") && 
     i < args.size() - 1 )
    {
    SRArgumentSpecified = true;
    this->RunConfigurationScript = true;
    i++;
    cmCTestScriptHandler* ch
      = static_cast<cmCTestScriptHandler*>(this->GetHandler("script"));
    ch->AddConfigurationScript(args[i].c_str(),true);
    }
  
  if(this->CheckArgument(arg, "-S", "--script") && i < args.size() - 1 )
    {
    this->RunConfigurationScript = true;
    i++;
    cmCTestScriptHandler* ch
      = static_cast<cmCTestScriptHandler*>(this->GetHandler("script"));
    // -SR is an internal argument, -S should be ignored when it is passed
    if (!SRArgumentSpecified)
      {
      ch->AddConfigurationScript(args[i].c_str(),true);
      }
    }
}

//----------------------------------------------------------------------
// the main entry point of ctest, called from main
int cmCTest::Run(std::vector<std::string> &args, std::string* output)
{
  this->FindRunningCMake();
  const char* ctestExec = "ctest";
  bool cmakeAndTest = false;
  bool performSomeTest = true;
  bool SRArgumentSpecified = false;

  // copy the command line
  for(size_t i=0; i < args.size(); ++i)
    {
    this->InitialCommandLineArguments.push_back(args[i]);
    }

  // process the command line arguments
  for(size_t i=1; i < args.size(); ++i)
    {
    // handle the simple commandline arguments
    this->HandleCommandLineArguments(i,args);

    // handle the script arguments -S -SR -SP
    this->HandleScriptArguments(i,args,SRArgumentSpecified);

    // handle a request for a dashboard
    std::string arg = args[i];
    if(this->CheckArgument(arg, "-D", "--dashboard") && i < args.size() - 1 )
      {
      this->ProduceXML = true;
      i++;
      std::string targ = args[i];
      // AddTestsForDashboard parses the dashborad type and converts it
      // into the seperate stages
      if (!this->AddTestsForDashboardType(targ))
        {
        performSomeTest = false;
        }
      }

    if(this->CheckArgument(arg, "-T", "--test-action") &&
      (i < args.size() -1) )
      {
      this->ProduceXML = true;
      i++;
      if ( !this->SetTest(args[i].c_str(), false) )
        {
        performSomeTest = false;
        cmCTestLog(this, ERROR_MESSAGE,
          "CTest -T called with incorrect option: "
          << args[i].c_str() << std::endl);
        cmCTestLog(this, ERROR_MESSAGE, "Available options are:" << std::endl
          << "  " << ctestExec << " -T all" << std::endl
          << "  " << ctestExec << " -T start" << std::endl
          << "  " << ctestExec << " -T update" << std::endl
          << "  " << ctestExec << " -T configure" << std::endl
          << "  " << ctestExec << " -T build" << std::endl
          << "  " << ctestExec << " -T test" << std::endl
          << "  " << ctestExec << " -T coverage" << std::endl
          << "  " << ctestExec << " -T memcheck" << std::endl
          << "  " << ctestExec << " -T notes" << std::endl
          << "  " << ctestExec << " -T submit" << std::endl);
        }
      }

    // what type of test model
    if(this->CheckArgument(arg, "-M", "--test-model") &&
      (i < args.size() -1) )
      {
      i++;
      std::string const& str = args[i];
      if ( cmSystemTools::LowerCase(str) == "nightly" )
        {
        this->SetTestModel(cmCTest::NIGHTLY);
        }
      else if ( cmSystemTools::LowerCase(str) == "continuous" )
        {
        this->SetTestModel(cmCTest::CONTINUOUS);
        }
      else if ( cmSystemTools::LowerCase(str) == "experimental" )
        {
        this->SetTestModel(cmCTest::EXPERIMENTAL);
        }
      else
        {
        performSomeTest = false;
        cmCTestLog(this, ERROR_MESSAGE,
          "CTest -M called with incorrect option: " << str.c_str()
          << std::endl);
        cmCTestLog(this, ERROR_MESSAGE, "Available options are:" << std::endl
          << "  " << ctestExec << " -M Continuous" << std::endl
          << "  " << ctestExec << " -M Experimental" << std::endl
                   << "  " << ctestExec << " -M Nightly" << std::endl);
        }
      }

    if(this->CheckArgument(arg, "--extra-submit") && i < args.size() - 1)
      {
      this->ProduceXML = true;
      this->SetTest("Submit");
      i++;
      if ( !this->SubmitExtraFiles(args[i].c_str()) )
        {
        return 0;
        }
      }

    // --build-and-test options
    if(this->CheckArgument(arg, "--build-and-test") && i < args.size() - 1)
      {
      cmakeAndTest = true;
      }

    // pass the argument to all the handlers as well, but i may no longer be
    // set to what it was originally so I'm not sure this is working as
    // intended
    cmCTest::t_TestingHandlers::iterator it;
    for ( it = this->TestingHandlers.begin();
      it != this->TestingHandlers.end();
      ++ it )
      {
      if ( !it->second->ProcessCommandLineArguments(arg, i, args) )
        {
        cmCTestLog(this, ERROR_MESSAGE,
          "Problem parsing command line arguments within a handler");
        return 0;
        }
      }
    } // the close of the for argument loop


  // now what sould cmake do? if --build-and-test was specified then 
  // we run the build and test handler and return
  if(cmakeAndTest)
    {
    this->Verbose = true;
    cmCTestBuildAndTestHandler* handler =
      static_cast<cmCTestBuildAndTestHandler*>(this->GetHandler("buildtest"));
    int retv = handler->ProcessHandler();
    *output = handler->GetOutput();
#ifdef CMAKE_BUILD_WITH_CMAKE
    cmDynamicLoader::FlushCache();
#endif
    if(retv != 0)
      {
      cmCTestLog(this, DEBUG, "build and test failing returing: " << retv 
                 << std::endl);
      }
    return retv;
    }

  // if some tests must be run 
  if(performSomeTest)
    {
    int res;
    // call process directory
    if (this->RunConfigurationScript)
      {
      if ( this->ExtraVerbose )
        {
        cmCTestLog(this, OUTPUT, "* Extra verbosity turned on" << std::endl);
        }
      cmCTest::t_TestingHandlers::iterator it;
      for ( it = this->TestingHandlers.begin();
        it != this->TestingHandlers.end();
        ++ it )
        {
        it->second->SetVerbose(this->ExtraVerbose);
        it->second->SetSubmitIndex(this->SubmitIndex);
        }
      this->GetHandler("script")->SetVerbose(this->Verbose);
      res = this->GetHandler("script")->ProcessHandler();
      if(res != 0)
        {
        cmCTestLog(this, DEBUG, "running script failing returning: " << res 
                   << std::endl);
        }

      }
    else
      {
      // What is this?  -V seems to be the same as -VV, 
      // and Verbose is always on in this case
      this->ExtraVerbose = this->Verbose;
      this->Verbose = true;
      cmCTest::t_TestingHandlers::iterator it;
      for ( it = this->TestingHandlers.begin();
        it != this->TestingHandlers.end();
        ++ it )
        {
        it->second->SetVerbose(this->Verbose);
        it->second->SetSubmitIndex(this->SubmitIndex);
        }
      if ( !this->Initialize(
          cmSystemTools::GetCurrentWorkingDirectory().c_str()) )
