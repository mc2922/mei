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

	struct report {
		double x;
		double y;
		double temp;
	};

	void requestSensor();
	void publishWaypoint(double xin, double yin);
	void publishSegList(vector<double> xin, vector<double> yin);

	string serializeParameter(string name, double parameter);
	report parseSensor(string msgIn);
	void parseAcomms(string msgIn);
	void tellMe(string varname, double parameter);

	double simpleAverage(vector<report> reportsIn);
	double simpleMidpoint(vector<report> reportsIn);

private:
	enum STATE {
		s_initial_scan = 0,
		s_estimate_T_S,
		s_estimate_offset,
		s_estimate_angle,
		s_acomms_ack,
		s_acomms_listening
	};
	STATE state;

	double id;
	double navx,navy;
	double tSent, timeout;
	string vname;
	string start;
	vector<report> unhandled_reports;
	vector<report> current_reports;
	vector<report> saved_reports;
	string latest_acomms;
	bool state_initialized, state_transit;
	bool heard_acomms;

	// Front parameters
	double offset;
	double east_offset;
	double angle;
	double amplitude;
	double period;
	double wavelength;
	double alpha;
	double beta;
	double T_N;
	double T_S;

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
};

#endif 
