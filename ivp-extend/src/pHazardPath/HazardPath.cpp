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
	id = 0;
	state = "init";
	myx=0;myy=0;
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
		if (key == "SEARCH_REGION") {
			string val = msg.GetString();
			//-150,-75:-150,-400:400,-400:400,-75
			MOOSChomp(val,"{");
			xmin = boost::lexical_cast<double>(MOOSChomp(val,","));
			ymax = boost::lexical_cast<double>(MOOSChomp(val,":"));
			MOOSChomp(val,",");
			ymin = boost::lexical_cast<double>(MOOSChomp(val,":"));
			xmax = boost::lexical_cast<double>(MOOSChomp(val,","));
			state = "loners";
		}
		else if(key=="NAV_X"){
			myx = msg.GetDouble();
		}
		else if(key=="NAV_Y"){
			myy = msg.GetDouble();
		}
		else if(key=="DEPLOY"){
			start_time=msg.GetTime();
		}
	}

	return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool HazardPath::Iterate()
{
	if(id==1){ //vehicle 1 - accurate
		if(MOOSTime()-start_time>=max_time){
			m_Comms.Notify("RETURN","true");
			state="mission_end";
		}
		else{
			if(state=="loners"){
				stringstream waypointsOut;
				vector<double> tempx,tempy;
				tempx.push_back(0); tempy.push_back(ymax-skew);
				tempx.push_back(xmax);tempy.push_back(ymax-skew);
				tempx.push_back(xmax); tempy.push_back(ymax-3*skew);
				tempx.push_back(0);tempy.push_back(ymax-3*skew);
				publishSegList(tempx,tempy);
			}
		}
	}

	else if(id==2){//vehicle 2 - wide
		if(MOOSTime()-start_time>=max_time){
			m_Comms.Notify("RETURN","true");
			state="mission_end";
		}
		else{
			if(state=="loners"){
				stringstream waypointsOut;
				vector<double> tempx,tempy;
				tempx.push_back(0); tempy.push_back(ymax-skew);
				tempx.push_back(xmin);tempy.push_back(ymax-skew);
				tempx.push_back(xmin); tempy.push_back(ymin+skew);
				tempx.push_back(xmin+2*skew);tempy.push_back(ymin+skew);
				tempx.push_back(xmin+2*skew);tempy.push_back(ymax-3*skew);
				publishSegList(tempx,tempy);
			}
		}
	}
	return true;
}

void HazardPath::publishSegList(vector<double> xin, vector<double> yin){
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
	m_Comms.Notify("TSP_UPDATES",waypointsOut.str());
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool HazardPath::OnStartUp()
{
	m_MissionReader.GetConfigurationParam("id", id);
	m_MissionReader.GetConfigurationParam("swath_width",swath_width);
	m_MissionReader.GetConfigurationParam("max_time",max_time);
	skew = swath_width-2;

	return true;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool HazardPath::OnConnectToServer()
{
	m_Comms.Register("SEARCH_REGION", 0);
	m_Comms.Register("NAV_X",0);
	m_Comms.Register("NAV_Y",0);
	m_Comms.Register("DEPLOY",0);
	m_Comms.Register("HAZARDSET_REQUEST", 0);

	return true;
}
