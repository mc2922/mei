/************************************************************/
/*    NAME: Mei                                               */
/*    ORGN: MIT                                             */
/*    FILE: PeriodicDriver.cpp                                        */
/*    DATE: 06/19/2018                                                */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "PeriodicDriver.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

using namespace std;

//---------------------------------------------------------
// Constructor

PeriodicDriver::PeriodicDriver()
{
  seconds = 0;
}

//---------------------------------------------------------
// Destructor

PeriodicDriver::~PeriodicDriver()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool PeriodicDriver::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();

#if 0 // Keep these around just for template
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString(); 
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif

     if(key == "FOO") 
       cout << "great!";

     else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
       reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool PeriodicDriver::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool PeriodicDriver::Iterate()
{
  AppCastingMOOSApp::Iterate();
  boost::posix_time::ptime timer(boost::posix_time::microsec_clock::local_time());
  seconds = timer.time_of_day().fractional_seconds();
  if(seconds<=900000){
    long sleep_duration = 900000-seconds;
    mode = "sleeping now";
    boost::this_thread::sleep_for(boost::chrono::microseconds(sleep_duration));
    m_Comms.Notify("ELLIPSE_UPDATES","speed=0.0");
    mode = "waiting now";
    boost::this_thread::sleep_for(boost::chrono::microseconds(100000));
    m_Comms.Notify("ELLIPSE_UPDATES","speed=1.0");
    mode = "driving";
  }else{
    mode = "something went wrong";
  }
  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool PeriodicDriver::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  for(p=sParams.begin(); p!=sParams.end(); p++) {
    string orig  = *p;
    string line  = *p;
    string param = toupper(biteStringX(line, '='));
    string value = line;

    bool handled = false;
    if(param == "FOO") {
      handled = true;
    }
    else if(param == "BAR") {
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);

  }
  
  registerVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables

void PeriodicDriver::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool PeriodicDriver::buildReport() 
{
  m_msgs << "============================================ \n";
  m_msgs << "File:                                        \n";
  m_msgs << "============================================ \n";

  ACTable actab(2);
  actab << "Seconds |  Mode ";
  actab.addHeaderLines();
  actab << (int)seconds << mode;
  m_msgs << actab.getFormattedString();

  return(true);
}




