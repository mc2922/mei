/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: AcousticSource.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "AcousticSource.h"

using namespace std;

//---------------------------------------------------------
// Constructor

AcousticSource::AcousticSource()
{
  mission_started = false;
  source_x = 0;
  source_y = 0;
  status_msg = "Initialized";
}

//---------------------------------------------------------
// Destructor

AcousticSource::~AcousticSource()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool AcousticSource::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

    string KEY = msg.GetKey();
      if (KEY == "MISSION_MODE") {
        string val = msg.GetString();
        if(val=="CIRCLE") {
          mission_started = true;
        }
        else {
          mission_started = false;
      }
    }

    if (KEY == "SOURCE_X") {
      source_x = msg.GetDouble();
      status_msg = "Heard: " + to_string(source_x);
      m_Comms.Notify("FOLLOW_STATUS", status_msg);
    }
    if(KEY == "SOURCE_Y") {
      source_y = msg.GetDouble();
    }
#if 0 // Keep these around just for template
    string key   = msg.GetKey();
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString();
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif
   }

   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool AcousticSource::OnConnectToServer()
{
   // register for variables here
   // possibly look at the mission file?
   // m_MissionReader.GetConfigurationParam("Name", <string>);
   // m_Comms.Register("VARNAME", 0);

   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool AcousticSource::Iterate()
{
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool AcousticSource::OnStartUp()
{
  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams)) {
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++) {
      string original_line = *p;
      string param = stripBlankEnds(toupper(biteString(*p, '=')));
      string value = stripBlankEnds(*p);

      if(param == "FOO") {
        //handled
      }
      else if(param == "BAR") {
        //handled
      }
    }
  }

  RegisterVariables();
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void AcousticSource::RegisterVariables()
{
  // Register("FOOBAR", 0);
}
