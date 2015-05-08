/*
 * mc2922
 *        File: FrontEstimate.cpp
 *  Created on: May 04, 2015
 *      Author: mc2922
 */

#include <iterator>
#include "MBUtils.h"
#include "FrontEstimate.h"

using namespace std;

//---------------------------------------------------------
// Constructor

FrontEstimate::FrontEstimate()
{

	anneal.clearMeas();
	cooling_steps = 100;
	anneal_step = 0;

	min_offset = -200;
	max_offset = 0;
	min_angle = -10;
	max_angle = 40;
	min_amplitude = 0;
	max_amplitude = 50;
	min_period = 60;
	max_period = 180;
	min_wavelength = 50;
	max_wavelength = 150;
	min_beta = 10;
	max_beta = 50;
	min_T_N = 15;
	max_T_N = 25;
	min_T_S = 20;
	max_T_S = 30;
	min_alpha = 500;
	max_alpha = 500;

	id = 1;
	start = "false";
	state = s_initial_scan;
	navx = 0; navy = 0;
	timeout = 10;
	tSent = 0;
	state_initialized = false;
	state_transit = true;
	heard_acomms = false;
	annealer_initialized = false;
	driver_ready = false;
	mlist.set_count(0);

	codec = goby::acomms::DCCLCodec::get();
	try {
		codec->validate<Measurement>();
	} catch (goby::acomms::DCCLException& e) {
		std::cout << "failed to validate hazard" << std::endl;
	}

	try {
		codec->validate<MeasurementList>();
	} catch (goby::acomms::DCCLException& e) {
		std::cout << "failed to validate list" << std::endl;
	}
}

//---------------------------------------------------------
// Destructor

FrontEstimate::~FrontEstimate()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool FrontEstimate::OnNewMail(MOOSMSG_LIST &NewMail)
{
	MOOSMSG_LIST::iterator p;

	for(p=NewMail.begin(); p!=NewMail.end(); p++) {
		CMOOSMsg &msg = *p;
		string key = msg.GetKey();

		if (key == "NAV_X") {
			navx = msg.GetDouble();
		}
		else if (key == "NAV_Y") {
			navy = msg.GetDouble();
		}
		else if (key == "FRONT_ESTIMATE_START") {
			start = msg.GetString();
			if(id==1){
				utc_time_offset = MOOSTime();
				m_Comms.Notify("UTC_TIME_OFFSET",utc_time_offset);
				unhandled_reports.clear();
			}
		}
		else if(key == "UCTD_SENSOR_REPORT"){
			string meas = msg.GetString();
			MOOSChomp(meas,"vname=");
			if(MOOSChomp(meas,",")==vname){
				if(state==s_annealing&&annealer_initialized){
					anneal.addMeas(anneal.parseMeas(msg.GetString()));
				}
				else
					unhandled_reports.push_back(parseSensor(msg.GetString()));
			}
		}

		else if(key == "GOTO_END"){
			state_transit = false;
		}
		else if(key == "ACOMMS_RECEIVED_DATA"){
			if(state==s_annealing&&annealer_initialized){
				mlist.Clear();
				codec->decode(msg.GetString(),&mlist);
				cout << "Received " << mlist.count() << " points from K" << endl;
				double count = mlist.count();
				for(int i=0;i<count;i++){
					stringstream measIn;
					Measurement meas = mlist.mlist(i);
					measIn << "utc=" << meas.t()<<",";
					measIn << "x=" << meas.x()<<",";
					measIn << "y=" << meas.y()<<",";
					measIn << "temp=" << meas.temp();
					anneal.addMeas(anneal.parseMeas(measIn.str()));
				}
			}
			else{
				parseAcomms(msg.GetString());
			}
		}
		else if(key == "ACOMMS_DRIVER_STATUS"){
			if(msg.GetDouble()==HoverAcomms::READY){
				driver_ready = true;
			}
			else{
				driver_ready = false;
			}
		}
		else if(key=="UTC_TIME_OFFSET"){
			cout << "Initialized utc offset" << endl;
			utc_time_offset = msg.GetDouble();
			unhandled_reports.clear();
		}
	}

	return(true);
}

bool FrontEstimate::Iterate()
{
	if(start=="true"){
		requestSensor();

		if(id==1){
			switch(state) {

			case s_initial_scan: //Travel North
				if(!state_initialized){
					publishWaypoint(160,navy);
					state_transit = true;
					state_initialized = true;
				}
				else if(!state_transit){
					cout << "Heard: " << unhandled_reports.size() << " reports" << endl;
					T_N = simpleAverage(unhandled_reports);
					unhandled_reports.clear();
					tellMe("T_N",T_N);

					latest_acomms = serializeParameter("T_N",T_N);
					m_Comms.Notify("ACOMMS_TRANSMIT_DATA",latest_acomms);
					tSent = MOOSTime();

					state = s_acomms_ack;
					state_transit = false;
					state_initialized = false;
				}
				break;

			case s_acomms_ack: //Ack first for ID1
				if(heard_acomms){
					state = s_acomms_periodic;
					state_initialized = false;
					heard_acomms = false;
					unhandled_reports.clear();
				}
				else if(MOOSTime()-tSent>timeout){
					m_Comms.Notify("ACOMMS_TRANSMIT_DATA",latest_acomms);
					tSent = MOOSTime();
				}
				break;

			case s_acomms_periodic: //Periodic sensor reports
				if(!state_initialized){
					tSent = MOOSTime();
					state_initialized = true;
				}
				else{
					if(MOOSTime()-tSent>10){
						if(driver_ready){
							cout << "Compiling message" << endl;
							while(!unhandled_reports.empty()){
								Measurement meas = unhandled_reports.back();
								mlist.add_mlist()->CopyFrom(meas);
								double count = mlist.count()+1;
								mlist.set_count(count);
								if(count==28){
									unhandled_reports.clear();
								}
								else{
									unhandled_reports.pop_back();
									double excess = unhandled_reports.size()+count-28;
									if(excess>0){ //discarding every other report
										unhandled_reports.pop_back();
									}
								}
							}
							cout << "Sending: " << mlist.count() << " measurements" << endl;
							string bytes;
							codec->encode(&bytes,mlist);
							m_Comms.Notify("ACOMMS_TRANSMIT_DATA_BINARY",(void *) bytes.data(), bytes.size());
							mlist.Clear();
							mlist.set_count(0);
							tSent = MOOSTime();
						}
						else{
							cout << "Driver was not ready!" << endl;
						}
					}
				}
				break;

			}
		}

		if(id==2){
			switch(state) {

			case s_initial_scan: //Estimate offset
				if(!state_initialized){
					publishWaypoint(-50,navy);
					state_transit = true;
					state_initialized = true;
					unhandled_reports.clear();
				}
				else if(!state_transit){
					cout << "Heard: " << unhandled_reports.size() << " reports" << endl;
					T_S = simpleAverage(unhandled_reports);
					unhandled_reports.clear();
					tellMe("T_S",T_S);
					latest_acomms = serializeParameter("T_S",T_S);

					state = s_acomms_listening;
				}
				break;

			case s_acomms_listening: //Listen first for ID 2
				if(heard_acomms){
					heard_acomms = false;
					m_Comms.Notify("ACOMMS_TRANSMIT_DATA",latest_acomms);
					state = s_annealing;
					state_initialized = false;
				}
				break;

			case s_annealing:
				if(!state_initialized){

					initializeAnnealer();
					cout << "Annealer Initialized" << endl;

					bhvZigzag("east",navx,170,navy,-45);
					state_transit = true;
					state_initialized = true;
					unhandled_reports.clear();
					tSent = MOOSTime();
				}
				else{
					double temperature = exp(-2*double(anneal_step)/double(cooling_steps));
					double energy = anneal.heatBath(temperature);

					if((MOOSTime()-tSent)>30){
						postParameterReport();
						tSent = MOOSTime();
					}

					if(!state_transit){
						bhvZigzag("west",navx,-50,navy,-45);
					}
				}
				break;
			}
		}
	}
	return true;
}

void FrontEstimate::bhvZigzag(string dir, double xmin, double xmax, double ymin, double ymax){

	vector<double> xvec,yvec;
	int cycles = 5;
	double xspan = (xmax-xmin)/cycles;
	if(dir=="east"){
		xvec.push_back(xmin);
		yvec.push_back(ymin);
		for(int i=1;i<cycles;i++){
			xvec.push_back(xmin+(i-0.5)*(xspan));
			xvec.push_back(xmin+i*(xspan));
			yvec.push_back(ymax);
			yvec.push_back(ymin);
		}
	}
	else if(dir=="west"){
		xvec.push_back(xmax);
		yvec.push_back(ymin);
		for(int i=1;i<cycles;i++){
			xvec.push_back(xmax-i*(xspan/2));
			xvec.push_back(xmax-i*(xspan));
			yvec.push_back(ymax);
			yvec.push_back(ymin);
		}
	}
	publishSegList(xvec,yvec);
}

void FrontEstimate::initializeAnnealer(){
	anneal.setVars(9, 1, true);
	vector<double> vars;
	vars.push_back(min_offset);
	vars.push_back(min_angle);
	vars.push_back(min_amplitude);
	vars.push_back(min_period);
	vars.push_back(min_wavelength);
	vars.push_back(min_alpha);
	vars.push_back(min_beta);
	vars.push_back(min_T_N);
	vars.push_back(min_T_S);
	anneal.setMinVal(vars);
	vars.clear();
	vars.push_back(max_offset);
	vars.push_back(max_angle);
	vars.push_back(max_amplitude);
	vars.push_back(max_period);
	vars.push_back(max_wavelength);
	vars.push_back(max_alpha);
	vars.push_back(max_beta);
	vars.push_back(max_T_N);
	vars.push_back(max_T_S);
	anneal.setMaxVal(vars);
	vars.clear();
	vars.push_back(0.5*(max_offset+min_offset));
	vars.push_back(0.5*(max_angle+min_angle));
	vars.push_back(0.5*(max_amplitude+min_amplitude));
	vars.push_back(0.5*(max_period+min_period));
	vars.push_back(0.5*(max_wavelength+min_period));
	vars.push_back(0.5*(max_alpha+min_alpha));
	vars.push_back(0.5*(max_beta+min_beta));
	vars.push_back(0.5*(max_T_N+min_T_N));
	vars.push_back(0.5*(max_T_S+min_T_S));
	anneal.setInitVal(vars);
	annealer_initialized = true;
}

void FrontEstimate::parseAcomms(string msgIn){
	string varname; double param;
	MOOSChomp(msgIn,"varname=");
	varname = MOOSChomp(msgIn,",");
	MOOSChomp(msgIn,"=");
	param = boost::lexical_cast<double>(MOOSChomp(msgIn,","));

	cout << "Heard: " << varname << " set to " << param << endl;

	if(varname=="T_N"){	// ID 1->2
		min_T_N = param;
		max_T_N = param;
		T_N = param;
	}
	else if(varname == "T_S"){
		min_T_S = param;
		max_T_S = param;
		T_S = param;
	}
	heard_acomms = true;
}

string FrontEstimate::serializeParameter(string name, double parameter){
	stringstream msgOut;
	msgOut << "varname=" << name << "," << "parameter=" << parameter;
	return msgOut.str();
}

double FrontEstimate::simpleAverage(vector<Measurement> reportsIn){
	double average = 0;
	for(int i=0;i<reportsIn.size();i++){
		Measurement tempreport = reportsIn[i];
		average = average+tempreport.temp();
	}
	average = average/reportsIn.size();
	return average;
}

void FrontEstimate::publishWaypoint(double xin, double yin){
	stringstream pointsOut, waypointsOut;
	pointsOut <<  "pts = {";
	waypointsOut << "points = ";
	pointsOut << navx <<","<< navy << ":";
	pointsOut << xin <<","<< yin << "}";
	waypointsOut << pointsOut.str();

	string seglist_label;
	if(id==1){
		seglist_label="edge_color=white,vertex_color=blue,vertex_size=10,edge_size=1";
	}
	else if(id==2){
		seglist_label="edge_color=pink,vertex_color=red,vertex_size=10,edge_size=1";
	}

	pointsOut << ",label=v"<< id <<","<<seglist_label;
	m_Comms.Notify("VIEW_SEGLIST",pointsOut.str());
	cout << waypointsOut.str() << endl;
	m_Comms.Notify("GOTO_UPDATES",waypointsOut.str());
	m_Comms.Notify("MISSION_MODE","GOTO");
}

void FrontEstimate::publishSegList(vector<double> xin, vector<double> yin){
	stringstream pointsOut, waypointsOut;
	pointsOut <<  "pts = {";
	waypointsOut << "points = ";
	for(int i=0;i<xin.size();i++){
		pointsOut << xin[i] <<","<< yin[i];
		if(i<xin.size()-1){
			pointsOut << ":";
		}
	}
	pointsOut << "}";
	waypointsOut << pointsOut.str();

	string seglist_label;
	if(id==1){
		seglist_label="edge_color=white,vertex_color=blue,vertex_size=10,edge_size=1";
	}
	else if(id==2){
		seglist_label="edge_color=pink,vertex_color=red,vertex_size=10,edge_size=1";
	}

	pointsOut << ",label=v"<< id <<","<<seglist_label;
	m_Comms.Notify("VIEW_SEGLIST",pointsOut.str());
	m_Comms.Notify("GOTO_UPDATES",waypointsOut.str());
	m_Comms.Notify("MISSION_MODE","GOTO");
}

void FrontEstimate::requestSensor(){
	string msgOut = "vname="+vname;
	m_Comms.Notify("UCTD_SENSOR_REQUEST",msgOut);
}

Measurement FrontEstimate::parseSensor(string msgIn){
	Measurement reportOut;
	MOOSChomp(msgIn,"utc=");
	reportOut.set_t(boost::lexical_cast<double>(MOOSChomp(msgIn,","))-utc_time_offset);
	MOOSChomp(msgIn,"x=");
	reportOut.set_x(boost::lexical_cast<double>(MOOSChomp(msgIn,",")));
	MOOSChomp(msgIn,"y=");
	reportOut.set_y(boost::lexical_cast<double>(MOOSChomp(msgIn,",")));
	MOOSChomp(msgIn,"temp=");
	reportOut.set_temp(boost::lexical_cast<double>(MOOSChomp(msgIn,",")));

	return reportOut;
}

void FrontEstimate::tellMe(string varname, double param){
	cout << varname << ": " << param << endl;

	if(varname=="T_N"){
		if(param > max_T_N)
			cout << "Exceeded max!" << endl;
		else if(param < min_T_N)
			cout << "Exceeded min!" << endl;
	}
	else if(varname=="T_S"){
		if(param > max_T_S)
			cout << "Exceeded max!" << endl;
		else if(param < min_T_S)
			cout << "Exceeded min!" << endl;
	}
}

void FrontEstimate::postParameterReport()
{
	vector<double> result;
	anneal.getEstimate(result);
	offset =     result[0];
	angle  =     result[1];
	amplitude =  result[2];
	period =     result[3];
	wavelength = result[4];
	alpha =      result[5];
	beta =       result[6];
	T_N  =       result[7];
	T_S  =       result[8];

	string sval;
	sval = "vname=" + vname;
	sval += ",offset=" + doubleToString(offset);
	sval += ",angle=" + doubleToString(angle);
	sval += ",amplitude=" + doubleToString(amplitude);
	sval += ",period=" + doubleToString(period);
	sval += ",wavelength=" + doubleToString(wavelength);
	sval += ",alpha=" + doubleToString(alpha);
	sval += ",beta=" + doubleToString(beta);
	sval += ",tempnorth=" + doubleToString(T_N);
	sval += ",tempsouth=" + doubleToString(T_S);

	cout << sval << endl;

	m_Comms.Notify("UCTD_PARAMETER_ESTIMATE", sval);
}

bool FrontEstimate::OnStartUp()
{
	m_MissionReader.GetConfigurationParam("id", id);
	m_MissionReader.GetConfigurationParam("vname", vname);

	m_MissionReader.GetConfigurationParam("cooling_steps", cooling_steps);

	//Front estimate parameters
	m_MissionReader.GetConfigurationParam("min_offset", min_offset);
	m_MissionReader.GetConfigurationParam("max_offset", max_offset);
	m_MissionReader.GetConfigurationParam("min_angle", min_angle);
	m_MissionReader.GetConfigurationParam("max_angle", max_angle);
	m_MissionReader.GetConfigurationParam("min_amplitude", min_amplitude);
	m_MissionReader.GetConfigurationParam("max_amplitude", max_amplitude);
	m_MissionReader.GetConfigurationParam("min_period", min_period);
	m_MissionReader.GetConfigurationParam("max_period", max_period);
	m_MissionReader.GetConfigurationParam("min_wavelength", min_wavelength);
	m_MissionReader.GetConfigurationParam("max_wavelength", max_wavelength);
	m_MissionReader.GetConfigurationParam("min_beta", min_beta);
	m_MissionReader.GetConfigurationParam("max_beta", max_beta);
	m_MissionReader.GetConfigurationParam("min_T_N", min_T_N);
	m_MissionReader.GetConfigurationParam("max_T_N", max_T_N);
	m_MissionReader.GetConfigurationParam("min_T_S", min_T_S);
	m_MissionReader.GetConfigurationParam("max_T_S", max_T_S);
	m_MissionReader.GetConfigurationParam("min_alpha", min_alpha);
	m_MissionReader.GetConfigurationParam("max_alpha", max_alpha);

	if(id==1){
	}
	else if(id==2){
		m_Comms.Register("UTC_TIME_OFFSET",0);
	}

	m_Comms.Notify("ACOMMS_TRANSMIT_RATE",1);

	return true;
}

bool FrontEstimate::OnConnectToServer()
{
	m_Comms.Register("FRONT_ESTIMATE_START",0);
	m_Comms.Register("NAV_X", 0);
	m_Comms.Register("NAV_Y", 0);
	m_Comms.Register("GOTO_END",0);
	m_Comms.Register("ACOMMS_RECEIVED_DATA",0);
	m_Comms.Register("UCTD_SENSOR_REPORT",0);
	m_Comms.Register("ACOMMS_DRIVER_STATUS",0);
	return true;
}
