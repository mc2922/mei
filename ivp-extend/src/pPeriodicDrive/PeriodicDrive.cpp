/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: PeriodicDrive.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "PeriodicDrive.h"

using namespace std;

//---------------------------------------------------------
// Constructor

PeriodicDrive::PeriodicDrive()
{
  drive_speed = 0.5;
}

//---------------------------------------------------------
// Destructor

PeriodicDrive::~PeriodicDrive()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool PeriodicDrive::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

   string key = msg.GetKey();
   if(key = "PERIODIC_DRIVE"){
     if(msg.GetString()=="TRUE"){
       mission_start = true;
     }
     else{
       mission_start = false;
     }
   }

   if(key = "PERIODIC_DRIVE_SPEED"){
     drive_speed = msg.GetDouble();
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

bool PeriodicDrive::OnConnectToServer()
{
   // register for variables here
   // possibly look at the mission file?
   // m_MissionReader.GetConfigurationParam("Name", <string>);
   // m_Comms.Register("VARNAME", 0);

   m_MissionReader.GetConfigurationParam("DRIVE_SPEED",drive_speed);

   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool PeriodicDrive::Iterate()
{
  if(mission_start){
    
  }

  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool PeriodicDrive::OnStartUp()
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

void PeriodicDrive::RegisterVariables()
{
  // Register("FOOBAR", 0);
}
