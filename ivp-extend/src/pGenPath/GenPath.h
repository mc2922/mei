/*
 * mc2922
 *        File: GenPath.h
 *  Created on: Mar 18, 2015
 *      Author: mc2922
 */

#ifndef GenPath_HEADER
#define GenPath_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <math.h>
#include <boost/lexical_cast.hpp>

class GenPath : public CMOOSApp
{
public:
    GenPath();
    ~GenPath();

protected:
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool Iterate();
    bool OnConnectToServer();
    bool OnStartUp();
    void solveTSP();
    double computeTSPDist(std::vector<double> xin, std::vector<double> yin);
    void publishSegList(std::vector<double> xin, std::vector<double> yin, std::string label, std::string vname);

private:
    std::vector<double> xvec, yvec;
    std::vector<int> indices;
    std::string vname,seglist_label;

    bool monitoring,deployed;
    double visit_radius,current_wpt, last_known;
    double start_x,start_y;
    double nav_x,nav_y;
};

#endif 
