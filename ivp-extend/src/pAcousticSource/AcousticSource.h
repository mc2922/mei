/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: AcousticSource.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef AcousticSource_HEADER
#define AcousticSource_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <string>

using namespace std;

class AcousticSource : public CMOOSApp
{
 public:
   AcousticSource();
   ~AcousticSource();

 protected: // Standard MOOSApp functions to overload
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected:
   void RegisterVariables();

 private: // Configuration variables
 bool mission_started;
 private: // State variables
 double source_x, source_y;
 string status_msg;
};

#endif
