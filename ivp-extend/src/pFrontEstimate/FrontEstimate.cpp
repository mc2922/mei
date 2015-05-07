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

	alpha = 500;

	id = 1;
	start = "false";
	state = s_initial_scan;
	navx = 0; navy = 0;
	timeout = 10;
	tSent = 0;
	state_initialized = false;
	state_transit = true;
	heard_acomms = false;
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
		}
		else if(key == "UCTD_SENSOR_REPORT"){
			unhandled_reports.push_back(parseSensor(msg.GetString()));
		}
		else if(key == "GOTO_END"){
			state_transit = false;
		}
		else if(key == "ACOMMS_RECEIVED_DATA"){
			parseAcomms(msg.GetString());
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
					publishWaypoint(160,10);
					state_transit = true;
					state_initialized = true;
				}
				else if(!state_transit){
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
					state = s_acomms_listening;
					heard_acomms = false;
				}
				else if(MOOSTime()-tSent>timeout){
					m_Comms.Notify("ACOMMS_TRANSMIT_DATA",latest_acomms);
					tSent = MOOSTime();
				}
				break;

			case s_acomms_listening: //Reply with ack for Id 1
				if(heard_acomms){
					heard_acomms = false;
					m_Comms.Notify("ACOMMS_TRANSMIT_DATA","varname=ack");
					state = s_estimate_offset;
				}
				break;

			case s_estimate_offset:
				if(!state_initialized){
					publishWaypoint(160,10);
					state_transit = true;
					cout << "Offset Estimate" << endl;
					state_initialized = true;
				}
				else if(!state_transit){
					east_offset = -200 - simpleMidpoint(unhandled_reports);
					unhandled_reports.clear();
					tellMe("east_offset",east_offset);

					angle = atan2((east_offset-offset),100);
					tellMe("angle",angle);
					latest_acomms = serializeParameter("angle",angle);
					state = s_acomms_ack;
				}
				break;
			}
		}

		if(id==2){
			switch(state) {

			case s_initial_scan: //Estimate offset
				if(!state_initialized){
					publishWaypoint(navx,-180);
					state_transit = true;
					state_initialized = true;
					unhandled_reports.clear();
				}
				else if(!state_transit){
					offset = simpleMidpoint(unhandled_reports);
					unhandled_reports.clear();
					tellMe("offset",offset);
					latest_acomms = serializeParameter("offset",offset);

					state = s_acomms_listening;
				}
				break;

			case s_acomms_listening: //LIsten first for ID 2
				if(heard_acomms){
					heard_acomms = false;
					m_Comms.Notify("ACOMMS_TRANSMIT_DATA",latest_acomms);
					state = s_acomms_ack;
				}
				break;

			case s_acomms_ack: //End ack chain for ID 2
				if(heard_acomms){
					heard_acomms = false;
					state = s_estimate_T_S;
				}
				break;

			case s_estimate_T_S:
				if(!state_initialized){
					publishWaypoint(-45,navy);
					state_transit = true;
					state_initialized = true;
					unhandled_reports.clear();
				}
				else if(!state_transit){
					T_S = simpleAverage(unhandled_reports);
					unhandled_reports.clear();
					tellMe("T_S",T_S);

					state = s_acomms_listening;
				}
				break;
			}
		}
	}
	return true;
}

void FrontEstimate::parseAcomms(string msgIn){
	string varname; double param;
	MOOSChomp(msgIn,"varname=");
	varname = MOOSChomp(msgIn,",");
	MOOSChomp(msgIn,"=");
	param = boost::lexical_cast<double>(MOOSChomp(msgIn,","));

	cout << "Heard: " << varname << " set to " << param << endl;

	if(varname=="T_N"){	// ID 1->2
		T_N = param;
	}
	else if(varname == "T_S"){
		T_S = param;
	}
	else if(varname == "offset"){
		offset = param;
	}
	heard_acomms = true;
}

string FrontEstimate::serializeParameter(string name, double parameter){
	stringstream msgOut;
	msgOut << "varname=" << name << "," << "parameter=" << parameter;
	return msgOut.str();
}

double FrontEstimate::simpleMidpoint(vector<report> reportsIn){
	double ymax,ymin;
	ymax = 20;
	ymin = -200;
	for(int i=0;i<reportsIn.size();i++){
		report myreport = reportsIn[i];
		if(abs(myreport.temp-T_N)<0.1){
			if(myreport.y < ymax){
				ymax = myreport.y;
			}
		}
		if(abs(myreport.temp-T_S)<0.1){
			if(myreport.y > ymin){
				ymin = myreport.y;
			}
		}
	}
	return (ymax-ymin)/2;
}

double FrontEstimate::simpleAverage(vector<report> reportsIn){
	double average = 0;
	for(int i=0;i<reportsIn.size();i++){
		report tempreport = reportsIn[i];
		average = average+tempreport.temp;
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

FrontEstimate::report FrontEstimate::parseSensor(string msgIn){
	report reportOut;
	MOOSChomp(msgIn,"x=");
	reportOut.x = boost::lexical_cast<double>(MOOSChomp(msgIn,","));
	MOOSChomp(msgIn,"y=");
	reportOut.y = boost::lexical_cast<double>(MOOSChomp(msgIn,","));
	MOOSChomp(msgIn,"temp=");
	reportOut.temp = boost::lexical_cast<double>(MOOSChomp(msgIn,","));

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

bool FrontEstimate::OnStartUp()
{
	m_MissionReader.GetConfigurationParam("id", id);
	m_MissionReader.GetConfigurationParam("vname", vname);

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
	return true;
}
