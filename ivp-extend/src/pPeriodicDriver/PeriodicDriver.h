/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: PeriodicDriver.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef PeriodicDriver_HEADER
#define PeriodicDriver_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class PeriodicDriver : public AppCastingMOOSApp
{
 public:
   PeriodicDriver();
   ~PeriodicDriver();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
   void registerVariables();

 private: // Configuration variables

 private: // State variables
};

#endif 
