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
    void solveTSP(int vehicle_id);
    double computeTSPDist(std::vector<double> xin, std::vector<double> yin);
    void publishSegList(std::vector<double> xin, std::vector<double> yin, std::string label, std::string vname);

private:
    std::vector<double> henry_xvec,henry_yvec;
    std::vector<double> gilda_xvec,gilda_yvec;
    std::vector<double> xsol,ysol,xtemp,ytemp;
    std::string vname,label;
	double visited_x,visited_y,current_obj;
	bool compute_henry,compute_gilda;

    double henry_x,henry_y;
    double gilda_x,gilda_y;
};

#endif 
