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
  mission = "IDLE";
  status_msg = "Init";
  source_x = -99;
  source_y = -99;
  nav_x = 0;
  nav_y = 0;
  goto_x_hist = 0;
  goto_y_hist = 0;
  radius = 10;
  bearing = 0;
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
        mission = msg.GetString();
    }

    if (KEY == "OFFSET_X"){
      offset_x = msg.GetDouble();
    }

    if (KEY == "OFFSET_Y"){
      offset_y = msg.GetDouble();
    }

    if (KEY == "CIRCLE_BEARING"){
      bearing = msg.GetDouble();
    }

    if (KEY == "CIRCLE_RADIUS"){
      bearing = msg.GetDouble();
    }

    if (KEY == "SOURCE_X") {
      source_x = msg.GetDouble();
      status_msg = "(Heard x) Current Source: " + to_string(source_x) + to_string(source_y);
      m_Comms.Notify("AS_STATUS", status_msg);
    }

    if(KEY == "SOURCE_Y") {
      source_y = msg.GetDouble();
      status_msg = "(Heard y) Current Source: " + to_string(source_x)  + to_string(source_y);
      m_Comms.Notify("AS_STATUS", status_msg);
    }

    if (KEY == "NAV_X"){
      nav_x = msg.GetDouble();
    }

    if (KEY == "NAV_Y"){
      nav_y = msg.GetDouble();
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


   m_MissionReader.GetConfigurationParam("offset_x",offset_x);
   m_MissionReader.GetConfigurationParam("offset_y",offset_y);
   m_MissionReader.GetConfigurationParam("bearing",bearing);
   m_MissionReader.GetConfigurationParam("radius",radius);

   RegisterVariables();

   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool AcousticSource::Iterate()
{
  if(mission=="AS_OFFSET"){
    goto_x = source_x + offset_x;
    goto_y = source_y + offset_y;

    if(goto_x != goto_x_hist || goto_y != goto_y_hist){
      status_msg = "points="+to_string(goto_x)+","+to_string(goto_y);
      m_Comms.Notify("GOTO_UPDATES",status_msg);
      goto_x_hist = goto_x;
      goto_y_hist = goto_y;
    }
  }

  if(mission=="AS_CIRCLE"){
    goto_x = source_x + radius*(cos(bearing/180*pi));
    goto_y = source_y + radius*(sin(bearing/180*pi));
    bearing += 1;

    if(goto_x != goto_x_hist || goto_y != goto_y_hist){
      status_msg = "points="+to_string(goto_x)+","+to_string(goto_y);
      m_Comms.Notify("GOTO_UPDATES",status_msg);
      goto_x_hist = goto_x;
      goto_y_hist = goto_y;
    }
  }

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
  Register("NAV_X",0);
  Register("NAV_Y",0);
  Register("SOURCE_X",0);
  Register("SOURCE_Y",0);
  Register("OFFSET_X",0);
  Register("OFFSET_Y",0);
  Register("MISSION_MODE",0);
  Register("CIRCLE_BEARING",0);
  Register("CIRCLE_RADIUS",0);
}
