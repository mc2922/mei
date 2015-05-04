/*
 * mc2922
 *        File: FrontEstimate.h
 *  Created on: May 04, 2015
 *      Author: mc2922
 */

#ifndef FrontEstimate_HEADER
#define FrontEstimate_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <boost/lexical_cast.hpp>

using namespace std;

class FrontEstimate : public CMOOSApp
{
public:
	FrontEstimate();
	~FrontEstimate();

protected:
	bool OnNewMail(MOOSMSG_LIST &NewMail);
	bool Iterate();
	bool OnConnectToServer();
	bool OnStartUp();

private:
	double id;
	double navx,navy;
	string start, state;

	// Front parameters
	double offset;
	double angle;
	double amplitude;
	double period;
	double wavelength;
	double alpha;
	double beta;
	double T_N;
	double T_S;
	double temperature;
	// lower limits for annealer
	double min_offset;
	double min_angle;
	double min_amplitude;
	double min_period;
	double min_wavelength;
	double min_alpha;
	double min_beta;
	double min_T_N;
	double min_T_S;
	// upper limits for annealer
	double max_offset;
	double max_angle;
	double max_amplitude;
	double max_period;
	double max_wavelength;
	double max_alpha;
	double max_beta;
	double max_T_N;
	double max_T_S;
	double delta_t;
};

#endif 
