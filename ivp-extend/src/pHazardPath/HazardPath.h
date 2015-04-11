/*
 * Mei Cheung
 *        File: HazardPath.h
 *  Created on: Apr 10, 2015
 *      Author: Mei Cheung
 */

#ifndef HazardPath_HEADER
#define HazardPath_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

class HazardPath : public CMOOSApp
{
public:
    HazardPath();
    ~HazardPath();

protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();

private:
};

#endif 
