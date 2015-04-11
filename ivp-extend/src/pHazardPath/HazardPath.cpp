/*
 * Mei Cheung
 *        File: HazardPath.cpp
 *  Created on: Apr 10, 2015
 *      Author: Mei Cheung
 */

#include <iterator>
#include "MBUtils.h"
#include "HazardPath.h"

using namespace std;

//---------------------------------------------------------
// Constructor

HazardPath::HazardPath()
{
}

//---------------------------------------------------------
// Destructor

HazardPath::~HazardPath()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool HazardPath::OnNewMail(MOOSMSG_LIST &NewMail)
{
    MOOSMSG_LIST::iterator p;

    for(p=NewMail.begin(); p!=NewMail.end(); p++) {
        CMOOSMsg &msg = *p;
        string key = msg.GetKey();
        if (key == "VARIABLE") {
            string val = msg.GetString();
        }
    }

    return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool HazardPath::OnConnectToServer()
{
    m_Comms.Register("VARIABLE", 0);
    return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool HazardPath::Iterate()
{
    return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool HazardPath::OnStartUp()
{
    string config_val;
    m_MissionReader.GetConfigurationParam("config_var", config_val);

    return true;
}

