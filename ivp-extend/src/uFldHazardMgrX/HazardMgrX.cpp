/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: HazardMgr.cpp                                        */
/*    DATE: Oct 26th 2012                                        */
/*                                                               */
/* This file is part of MOOS-IvP                                 */
/*                                                               */
/* MOOS-IvP is free software: you can redistribute it and/or     */
/* modify it under the terms of the GNU General Public License   */
/* as published by the Free Software Foundation, either version  */
/* 3 of the License, or (at your option) any later version.      */
/*                                                               */
/* MOOS-IvP is distributed in the hope that it will be useful,   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty   */
/* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See  */
/* the GNU General Public License for more details.              */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with MOOS-IvP.  If not, see                     */
/* <http://www.gnu.org/licenses/>.                               */
/*****************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "HazardMgrX.h"
#include "XYFormatUtilsHazard.h"
#include "XYFormatUtilsPoly.h"
#include "ACTable.h"

using namespace std;

//---------------------------------------------------------
// Constructor

HazardMgrX::HazardMgrX()
{
	// Config variables
	m_swath_width_desired = 25;
	m_pd_desired          = 0.9;

	// State Variables
	m_sensor_config_requested = false;
	m_sensor_config_set   = false;
	m_swath_width_granted = 0;
	m_pd_granted          = 0;

	m_sensor_config_reqs = 0;
	m_sensor_config_acks = 0;
	m_sensor_report_reqs = 0;
	m_detection_reports  = 0;

	m_summary_reports = 0;

	codec = goby::acomms::DCCLCodec::get();
	try {
		codec->validate<Hazard>();
	} catch (goby::acomms::DCCLException& e) {
		std::cout << "failed to validate hazard" << std::endl;
	}

	try {
		codec->validate<HazardList>();
	} catch (goby::acomms::DCCLException& e) {
		std::cout << "failed to validate list" << std::endl;
	}

	hlist.set_count(0);
	myeast_it = 0;
	mywest_it = 0;
	coopeast_it = 0;
	coopwest_it = 0;
	state = "mission_start";
	start_time = -1;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool HazardMgrX::OnNewMail(MOOSMSG_LIST &NewMail)
{
	AppCastingMOOSApp::OnNewMail(NewMail);

	MOOSMSG_LIST::iterator p;
	for(p=NewMail.begin(); p!=NewMail.end(); p++) {
		CMOOSMsg &msg = *p;
		string key   = msg.GetKey();
		string sval  = msg.GetString();

#if 0 // Keep these around just for template
		string comm  = msg.GetCommunity();
		double dval  = msg.GetDouble();
		string msrc  = msg.GetSource();
		double mtime = msg.GetTime();
		bool   mdbl  = msg.IsDouble();
		bool   mstr  = msg.IsString();
#endif

		if(key == "UHZ_DETECTION_REPORT"){
			m_detection_reports++;

			MOOSChomp(sval,"=");
			double tx = boost::lexical_cast<double>(MOOSChomp(sval,","));
			MOOSChomp(sval,"=");
			double ty = boost::lexical_cast<double>(MOOSChomp(sval,","));
			MOOSChomp(sval,"=");

			stringstream event;

			if(sval == "") {
				reportRunWarning("Detection report received for hazard w/out label");
				return(false);
			}

			int tlabel = boost::lexical_cast<int>(sval);
			vector<int>::iterator it = find(labelset.begin(), labelset.end(), tlabel);
			if(it != labelset.end()){
				dset[distance(labelset.begin(),it)]++;
				event<<"Repeat Detection, label=";
			}
			else{
				xset.push_back(tx);yset.push_back(ty);
				labelset.push_back(tlabel);
				typeset.push_back("hazard");
				dset.push_back(1);
				cset.push_back(0);

				if(id==2){
					Hazard haz;
					haz.set_x(tx);
					haz.set_y(ty);
					hlist.add_list()->CopyFrom(haz);
					int tcount=hlist.count();
					if(hlist.count()<41){
					tcount++;}
					hlist.set_count(tcount);
				}
				event<<"New Detection, label=";
			}

			event<<tlabel<<", x="<<tx<<", y="<<ty;
			reportEvent(event.str());

			string req = "vname=" + m_host_community + ",label=" + sval;
			Notify("UHZ_CLASSIFY_REQUEST", req);
		}

		else if(key == "UHZ_HAZARD_REPORT"){
			MOOSChomp(sval,",");MOOSChomp(sval,",");
			string what = MOOSChomp(sval,"=");

			if(what=="type"){
				string ttype = MOOSChomp(sval,",");
				MOOSChomp(sval,"=");
				int tlabel = boost::lexical_cast<int>(sval);
				vector<int>::iterator it = find(labelset.begin(), labelset.end(), tlabel);
				typeset[distance(labelset.begin(),it)]=ttype;
				cset[distance(labelset.begin(),it)]++;
			}
			else if(what=="label"){
				int tlabel = boost::lexical_cast<int>(MOOSChomp(sval,","));
				MOOSChomp(sval,"=");
				string ttype = sval;
				vector<int>::iterator it = find(labelset.begin(), labelset.end(), tlabel);
				typeset[distance(labelset.begin(),it)]=ttype;
				cset[distance(labelset.begin(),it)]++;
			}
			else{
				reportRunWarning("Classification report without type or label!");
				return(false);
			}
		}

		else if(key=="TSP_END"){
			if(sval=="true"){
				if((state=="transit"||state=="cooptransit")){
					if(id==1){
						state = "waiting";
					}
					else if(id==2){
						if(hlist.count()>4){
							cout << "sending "<<hlist.count() << " points" << endl;
							string bytes;
							codec->encode(&bytes,hlist);
							//string message ="src_node=kasper,dest_node=jake,var_name=INTERVEHICLE_MESSAGE,string_val=";
							//message+=bytes;
							//cout << "Sent : " << bytes.size() << endl;
							//m_Comms.Notify("NODE_MESSAGE_LOCAL",message);
							m_Comms.Notify("INTERVEHICLE_MESSAGE_BINARY",(void *) bytes.data(), bytes.size());
							state = "coop";
							hlist.Clear();
							hlist.set_count(0);
						}
						else{
							stringstream msgOut;
							msgOut << myeast_it <<",";
							msgOut << mywest_it <<",";
							msgOut << type;
							m_Comms.Notify("INTERVEHICLE_MESSAGE",msgOut.str());
							state="loners";
						}
					}
				}
				else if(state=="heard"){
					state = "loners";
				}
				else if(state=="heardend"){
					state = "tsp";
				}
				else if(state=="tsptransit"){
					state = "tsp";
				}
			}
		}

		else if(key=="INTERVEHICLE_MESSAGE"){
			if(id==1){
				if(sval!="end"){
					coopeast_it = boost::lexical_cast<int>(MOOSChomp(sval,","));
					coopwest_it = boost::lexical_cast<int>(MOOSChomp(sval,","));
					if(state=="waiting"){
						state = "loners";
						type = sval;
						switchType();
					}
					else if(state=="transit"){
						state = "heard";
					}
				}
				else{
					if(state=="waiting"){
						state = "tsp";
					}
					else if(state=="transit"){
						state = "heardend";
					}
				}
			}
		}

		else if(key=="INTERVEHICLE_MESSAGE_BINARY"){
			if(id==1){
				hlist.Clear();
				codec->decode(sval,&hlist);
				cout << "Received " << hlist.count() << " points from K" << endl;
				state = "coop";
			}
		}

		else if(key=="NAV_X")
			myx = msg.GetDouble();

		else if(key=="NAV_Y")
			myy = msg.GetDouble();

		else if(key=="DEPLOY"){
			if(sval=="true"){
				cout << "heard deploy" << endl;
				start_time=msg.GetTime();
				state = "loners";
			}
		}

		else if(key == "UHZ_CONFIG_ACK")
			handleMailSensorConfigAck(sval);

		else if(key == "UHZ_OPTIONS_SUMMARY")
			handleMailSensorOptionsSummary(sval);

		else if(key == "UHZ_MISSION_PARAMS")
			handleMailMissionParams(sval);

		else if(key == "HAZARDSET_REQUEST")
			if(id==1){postHazardSetReport();}

		else
			reportRunWarning("Unhandled Mail: " + key);
	}

	return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool HazardMgrX::Iterate()
{
	//	AppCastingMOOSApp::Iterate();

	if(!m_sensor_config_requested)
		postSensorConfigRequest();

	if(m_sensor_config_set)
		postSensorInfoRequest();

	if(start_time==-1){return(true);}
	if(MOOSTime()-start_time>=max_time){
		m_Comms.Notify("STATION","true");

		if(id==1){
			postHazardSetReport();
		}
		return(true);
	}

	if(id==1){ //vehicle 1 - accurate
		if(state=="loners"){
			segmentSpace();
			state="transit";
			switchType();
		}
		else if(state=="coop"){
			readHazardList();
			solveTSP();
			if(hlist.count()>4){state="cooptransit";}
			else{
				state="tsp";
				xvec=xset;
				yvec=yset;
			}
		}
		else if(state=="tsp"){
			xvec=xset;
			yvec=yset;
			solveTSP();
			state = "tsptransit";
		}
	}

	else if(id==2){//vehicle 2 - wide
		if(state=="loners"){
			segmentSpace();
			state="transit";
			switchType();
		}
		else if(state=="coop"){
			segmentSpace();
			state="transit";
			switchType();
		}
	}

	//AppCastingMOOSApp::PostReport();
	return(true);
}

void HazardMgrX::segmentSpace(){
	vector<double> tempx,tempy;

	cout << "meast: "<<myeast_it<<" mwest: "<<mywest_it << " ceast: " << coopeast_it << " cwest: " << coopwest_it << endl;
	//cout<<"type="<<type<<" xs="<<xmin<<" xm="<<xmax<<" ys="<<ymin<<" ym="<<ym<<endl;
	if(type=="east"){
		ym = ymax - myeast_it*4*skew - coopeast_it*4*skew_coop;
		tempx.push_back((xmin+xmax)/2+5); tempy.push_back(ym-skew);
		tempx.push_back(xmax);tempy.push_back(ym-skew);
		tempx.push_back(xmax); tempy.push_back(ym-3*skew);
		tempx.push_back((xmax+xmin)/2+5); tempy.push_back(ym-3*skew);
		myeast_it++;
		coopwest_it++;

		if(id==2&&state=="coop"){coopwest_it--;}
	}
	else if(type=="west"){
		ym = ymax - mywest_it*4*skew - coopwest_it*4*skew_coop;
		tempx.push_back((xmax+xmin)/2-5); tempy.push_back(ym-skew);
		tempx.push_back(xmin);tempy.push_back(ym-skew);
		tempx.push_back(xmin);tempy.push_back(ym-3*skew);
		tempx.push_back((xmax+xmin)/2-5); tempy.push_back(ym-3*skew);
		mywest_it++;
		coopeast_it++;

		if(id==2&&state=="coop"){coopeast_it--;}
	}

	if(ym-ymin>-10){
		publishSegList(tempx,tempy);
	}
	else{
		if(id==1){
			state="tsp";
			xvec=xset;
			yvec=yset;
		}
		else if(id==2){
			m_Comms.Notify("STATION","true");
			state = "mission_end";
			cout << "sending "<<hlist.count() << " points" << endl;
			string bytes;
			codec->encode(&bytes,hlist);
			m_Comms.Notify("INTERVEHICLE_MESSAGE_BINARY",(void *) bytes.data(), bytes.size());
		}
	}
}

void HazardMgrX::switchType(){
	if(type=="east")
		type="west";
	else if(type=="west")
		type="east";
}

void HazardMgrX::readHazardList(){
	xvec.clear();yvec.clear();

	//Generate list of points from HazardList
	for(int i=0;i<hlist.count();i++){
		double x1,x2,y1,y2;
		bool found = false;
		x1 = hlist.list(i).x();
		y1 = hlist.list(i).y();

		for(int i=0;i<xset.size();i++){
			double dist = sqrt(pow(x1-xset[i],2)+pow(y1-yset[i],2));
			if(dist<2){found=true;}
		}

		if(!found){
			xvec.push_back(x1);
			yvec.push_back(y1);
		}
	}
}

void HazardMgrX::solveTSP(){
	//Use the following heuristic to generate a path:
	//Greedy 2-Opt (Guarantee 2-approx of opt)
	//Construct an initial solution greedily (closest point)
	double current_obj=0;
	std::vector<double> xsol,ysol,xtemp,ytemp;
	double visited_x,visited_y;
	double current_max = -1;
	double current_x,current_y;
	int current_ind;

	cout << "Start Computing" << endl;
	visited_x = myx; visited_y = myy;
	xsol.push_back(visited_x); ysol.push_back(visited_y);
	xtemp=xvec; ytemp=yvec;

	while(!xtemp.empty()){
		current_max = -1;
		current_x=0; current_y=0;
		current_ind=0;
		for(int i=0;i<xtemp.size();i++){
			double dist = sqrt(pow(xtemp[i]-visited_x,2)+pow(ytemp[i]-visited_y,2));
			if(current_max<0 || dist<current_max){
				current_max = dist;
				current_x = xtemp[i]; current_y = ytemp[i];
				current_ind = i;
			}
		}
		if(current_max>0){
			current_obj+=current_max;
			xsol.push_back(current_x); ysol.push_back(current_y);
			visited_x = current_x; visited_y = current_y;
			xtemp.erase(xtemp.begin()+current_ind);
			ytemp.erase(ytemp.begin()+current_ind);
		}
	}

	cout << "Improving Solution" << endl;
	xtemp=xsol; ytemp=ysol;

	//Improvement step - make pairwise exchanges
	bool improving = true;
	int i = 1; int j = i+1;
	double improv_obj = 0;
	while(improving){
		xtemp[i]=xtemp[j]; ytemp[i]=ytemp[j];
		xtemp[j]= xsol[i]; ytemp[j]=ysol[i];
		improv_obj = computeTSPDist(xtemp,ytemp);
		if(improv_obj>=current_obj){ //no improvement
			if(j<xsol.size()-1){
				xtemp = xsol; ytemp = ysol; //reset
				j=j+1;
			}else if (j==xsol.size()-1 && i<xsol.size()-2){
				xtemp = xsol; ytemp = ysol; //reset
				i=i+1;
				j=i+1;
			}else{//Terminate
				improving = false;	//Finished improving
			}
		}
		else{
			current_obj = improv_obj; //improve objective
			xsol = xtemp; ysol = ytemp; //accept exchange
			i=1; //reset
			j=i+1;
		}
	}
	if(hlist.count()>4){
		double xd = (xmax+xmin)/2-xsol[xsol.size()];
		double yd = (ymax+ymin)/2-ysol[ysol.size()];
		double cycles = (current_obj+sqrt(pow(xd,2)+pow(yd,2)))/550;
		xsol.push_back((xmax+xmin)/2);
		ysol.push_back(ymax-4*mywest_it*skew-coopwest_it*4*skew_coop-cycles*4*skew_coop);
	}
	publishSegList(xsol,ysol);	//Final answer
	cout << "Finished Computing" << endl;
}

double HazardMgrX::computeTSPDist(vector<double> xin, vector<double> yin){
	double total_distance=0;
	for(int i=1;i<xin.size();i++){
		total_distance += sqrt(pow(xin[i-1]-xin[i],2)+pow(yin[i-1]-yin[i],2));
	}
	return total_distance;
}

void HazardMgrX::publishSegList(vector<double> xin, vector<double> yin){
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

	pointsOut <<",label="<<m_report_name<<"_waypt," << seglist_label;
	//m_Comms.Notify("VIEW_SEGLIST",pointsOut.str());
	m_Comms.Notify("TSP_UPDATES",waypointsOut.str());
	m_Comms.Notify("PATHED","true");
	m_Comms.Notify("TSP_END","false");
	m_Comms.Notify("STATION","false");
}

void HazardMgrX::postHazardSetReport()
{
	stringstream report;
	report <<"source="<<m_host_community<<"#";

	for(int i=0;i<xset.size();i++){
		if(typeset[i]=="hazard"){
			report<<"x="<<xset[i]<<",";
			report<<"y="<<yset[i]<<",";
			report<<"label="<<labelset[i]<<"#";
		}
	}

	m_Comms.Notify("HAZARDSET_REPORT", report.str());
}

//---------------------------------------------------------
//UHZ_MISSION_PARAMS = "penalty_missed_hazard=150,penalty_false_alarm=25,max_time=600,
//                       penalty_max_time_over=300,penalty_max_time_rate=0.5,
//                       search_region=pts={-150,-75:-150,-400:400,-400:400,-75}"

void HazardMgrX::handleMailMissionParams(string str)
{
	vector<string> svector = parseStringZ(str, ',', "{");
	unsigned int i, vsize = svector.size();
	for(i=0; i<vsize; i++) {
		string param = biteStringX(svector[i], '=');
		string value = svector[i];

		if(param=="search_region"){
			//-150,-75:-150,-400:400,-400:400,-75
			MOOSChomp(value,"{");
			xmin = boost::lexical_cast<double>(MOOSChomp(value,","));
			ymax = boost::lexical_cast<double>(MOOSChomp(value,":"));
			MOOSChomp(value,",");
			ymin = boost::lexical_cast<double>(MOOSChomp(value,":"));
			xmax = boost::lexical_cast<double>(MOOSChomp(value,","));
		}
	}
}

bool HazardMgrX::OnStartUp()
{
	AppCastingMOOSApp::OnStartUp();

	m_MissionReader.GetConfigurationParam("swath_width",m_swath_width_desired);
	skew = m_swath_width_desired-1;
	double swath_width_coop;
	m_MissionReader.GetConfigurationParam("swath_width_coop",swath_width_coop);
	skew_coop = swath_width_coop-1;
	m_MissionReader.GetConfigurationParam("sensor_pd",m_pd_desired);
	m_MissionReader.GetConfigurationParam("report_name",m_report_name);

	m_MissionReader.GetConfigurationParam("id", id);
	if(id==1){type="east";}
	else{type="west";}
	m_MissionReader.GetConfigurationParam("max_time",max_time);

	AppCastingMOOSApp::RegisterVariables();

	return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool HazardMgrX::OnConnectToServer()
{
	m_Comms.Register("UHZ_DETECTION_REPORT", 0);
	m_Comms.Register("UHZ_HAZARD_REPORT",0);
	m_Comms.Register("UHZ_CONFIG_ACK", 0);
	m_Comms.Register("UHZ_OPTIONS_SUMMARY", 0);
	m_Comms.Register("UHZ_MISSION_PARAMS", 0);
	m_Comms.Register("HAZARDSET_REQUEST", 0);

	m_Comms.Register("NAV_X",0);
	m_Comms.Register("NAV_Y",0);
	m_Comms.Register("DEPLOY",0);
	m_Comms.Register("TSP_END",0);
	m_Comms.Register("INTERVEHICLE_MESSAGE",0);
	m_Comms.Register("INTERVEHICLE_MESSAGE_BINARY",0);

	return(true);
}

//Unchanged code from HazardMgr

void HazardMgrX::postSensorConfigRequest()
{
	string request = "vname=" + m_host_community;

	request += ",width=" + doubleToStringX(m_swath_width_desired,2);
	request += ",pd="    + doubleToStringX(m_pd_desired,2);

	m_sensor_config_requested = true;
	m_sensor_config_reqs++;
	Notify("UHZ_CONFIG_REQUEST", request);
}

void HazardMgrX::postSensorInfoRequest()
{
	string request = "vname=" + m_host_community;

	m_sensor_report_reqs++;
	Notify("UHZ_SENSOR_REQUEST", request);
}

bool HazardMgrX::handleMailSensorConfigAck(string str)
{
	// Expected ack parameters:
	string vname, width, pd, pfa, pclass;

	// Parse and handle ack message components
	bool   valid_msg = true;
	string original_msg = str;

	vector<string> svector = parseString(str, ',');
	unsigned int i, vsize = svector.size();
	for(i=0; i<vsize; i++) {
		string param = biteStringX(svector[i], '=');
		string value = svector[i];

		if(param == "vname")
			vname = value;
		else if(param == "pd")
			pd = value;
		else if(param == "width")
			width = value;
		else if(param == "pfa")
			pfa = value;
		else if(param == "pclass")
			pclass = value;
		else
			valid_msg = false;
	}

	if((vname=="")||(width=="")||(pd=="")||(pfa=="")||(pclass==""))
		valid_msg = false;

	if(!valid_msg)
		reportRunWarning("Unhandled Sensor Config Ack:" + original_msg);

	if(valid_msg) {
		m_sensor_config_set = true;
		m_sensor_config_acks++;
		m_swath_width_granted = atof(width.c_str());
		m_pd_granted = atof(pd.c_str());
	}

	return(valid_msg);
}

bool HazardMgrX::buildReport()
{
	m_msgs << "Config Requested:"                                  << endl;
	m_msgs << "    swath_width_desired: " << m_swath_width_desired << endl;
	m_msgs << "             pd_desired: " << m_pd_desired          << endl;
	m_msgs << "   config requests sent: " << m_sensor_config_reqs  << endl;
	m_msgs << "                  acked: " << m_sensor_config_acks  << endl;
	m_msgs << "------------------------ "                          << endl;
	m_msgs << "Config Result:"                                     << endl;
	m_msgs << "       config confirmed: " << boolToString(m_sensor_config_set) << endl;
	m_msgs << "    swath_width_granted: " << m_swath_width_granted << endl;
	m_msgs << "             pd_granted: " << m_pd_granted          << endl << endl;
	m_msgs << "--------------------------------------------" << endl << endl;

	m_msgs << "               sensor requests: " << m_sensor_report_reqs << endl;
	m_msgs << "             detection reports: " << m_detection_reports  << endl << endl;

	m_msgs << "      Hazardset Reports Posted: " << m_summary_reports << endl;
	m_msgs << "                   Report Name: " << m_report_name << endl;

	return(true);
}
