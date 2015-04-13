/*
 * Mei Cheung
 *        File: HazardPath.h
 *  Created on: Apr 10, 2015
 *      Author: Mei Cheung
 */

#ifndef HazardPath_HEADER
#define HazardPath_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <math.h>
#include <boost/lexical_cast.hpp>

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
    void publishSegList(std::vector<double> xin, std::vector<double> yin);

private:
    int id;
    double myx,myy;
    double xmin,xmax,ymin,ymax;
    double swath_width, skew;
    double start_time,max_time;

    std::string state;

};

#endif 
