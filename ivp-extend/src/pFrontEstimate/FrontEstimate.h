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
#include <../lib_henrik_util/CSimAnneal.h>
#include <../lib_henrik_util/CFrontSim.h>
#include "Measurement.pb.h"
#include "goby/acomms/dccl.h"
#include "HoverAcomms.h"

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

	CSimAnneal anneal;

	//utility
	void requestSensor();
	double simpleAverage(vector<Measurement> reportsIn);
	void postParameterReport();
	void initializeAnnealer();

	//behaviors
	void bhvZigzag(string dir, double xmin, double xmax, double ymin, double ymax);
	void publishWaypoint(double xin, double yin);
	void publishSegList(vector<double> xin, vector<double> yin);

	//parsers
	string serializeParameter(string name, double parameter);
	Measurement parseSensor(string msgIn);
	void parseAcomms(string msgIn);
	void tellMe(string varname, double parameter);


private:
	enum STATE {
		s_initial_scan = 0,
		s_estimate_T_S,
		s_estimate_angle,
		s_acomms_ack,
		s_acomms_listening,
		s_acomms_periodic,
		s_annealing
	};
	STATE state;

	double id;
	double navx,navy;
	double tSent, timeout, missionStart;
	string vname,start;
	vector<Measurement> unhandled_reports;
	string latest_acomms;
	bool state_initialized, state_transit, utc_initialized;
	bool heard_acomms;
	bool driver_ready;
	double utc_time_offset;

	goby::acomms::DCCLCodec* codec;
	MeasurementList mlist;

	double mission_length;
	//Annealer parameters
	int cooling_steps;
	int anneal_step;
	bool annealer_initialized;
	double temp_fac;

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
