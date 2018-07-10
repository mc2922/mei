/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: PeriodicDrive.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef PeriodicDrive_HEADER
#define PeriodicDrive_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <chrono>

class PeriodicDrive : public CMOOSApp
{
 public:
   PeriodicDrive();
   ~PeriodicDrive();

 protected: // Standard MOOSApp functions to overload
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected:
   void RegisterVariables();

 private: // Configuration variables
 double drive_speed; //DRIVE_SPEED

 private: // State variables
 std::chrono::high_resolution_clock::time_point t1,t2;
 std::chrono::duration<double> time_span;
 bool mission_start;
};

#endif
