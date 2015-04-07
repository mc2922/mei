/*
 * mc2922
 *        File: GenPath.cpp
 *  Created on: Mar 18, 2015
 *      Author: mc2922
 */

#include <iterator>
#include "MBUtils.h"
#include "GenPath.h"

using namespace std;

//---------------------------------------------------------
// Constructor

GenPath::GenPath()
{
	monitoring = false;
	deployed = false;
	nav_x=0;nav_y=0;
	visit_radius=5;
	start_x=0;start_y=0;
	current_wpt=-1;last_known=-1;
}

//---------------------------------------------------------
// Destructor

GenPath::~GenPath()
{

}

//---------------------------------------------------------
// Procedure: OnNewMail

bool GenPath::OnNewMail(MOOSMSG_LIST &NewMail)
{
	MOOSMSG_LIST::iterator p;

	for(p=NewMail.begin(); p!=NewMail.end(); p++) {
		CMOOSMsg &msg = *p;
		string key = msg.GetKey();
		if (key == "VISIT_POINT") {
			string val = msg.GetString();
			string mysub = MOOSChomp(val,","); //chomp out x
			if(mysub=="firstpoint"){
				//Do nothing
			}else if(mysub=="lastpoint"){
				//Finished
				solveTSP();
			}else{
				MOOSChomp(mysub,"=");
				xvec.push_back(boost::lexical_cast<double>(mysub));
				mysub = MOOSChomp(val,","); //Chomp ystr
				MOOSChomp(mysub,"=");
				yvec.push_back(boost::lexical_cast<double>(mysub));
			}
		}
		else if(key == "NAV_X"){
			nav_x = msg.GetDouble();
			if(monitoring&&deployed&&(last_known!=current_wpt)&&current_wpt>=0){
				double dist = sqrt(pow(xvec[current_wpt]-nav_x,2)+pow(yvec[current_wpt]-nav_y,2));
				if(dist<=visit_radius){
					indices.push_back(current_wpt);
					cout << current_wpt << " visited" <<endl;
					last_known = current_wpt; //Remember visit
				}
			}
		}
		else if(key == "NAV_Y"){
			nav_y = msg.GetDouble();
			if(monitoring&&deployed&&(last_known!=current_wpt)&&current_wpt>=0){
				double dist = sqrt(pow(xvec[current_wpt]-nav_x,2)+pow(yvec[current_wpt]-nav_y,2));
				if(dist<=visit_radius){
					indices.push_back(current_wpt);
					cout << current_wpt << " visited" <<endl;
					last_known = current_wpt; //Remember visit
				}
			}
		}
		else if(key == "GENPATH_REGENERATE"){
			string dstr = msg.GetString();
			if(dstr=="true"){
				double missed = xvec.size()-indices.size()-1;
				cout << "Missed points: " << missed << endl;
				if(missed>0){
					cout << "Regenerating"<<endl;
					vector<double> xm; vector<double> ym;
					int compare = 0; int original = 0;
					while(compare<indices.size()){
						if(original==indices[compare]){
							compare++;
							original++;
						}else{
							xm.push_back(xvec[original]);
							ym.push_back(yvec[original]);
							cout << "Missed: " << original <<endl;
							original++;
						}
					}
					publishSegList(xm,ym,"_regen",vname);	//Final answer
					xvec = xm; yvec = ym;
					m_Comms.Notify("STATION_KEEP","false");
					indices.clear();current_wpt=-1;last_known=-1; //Reset monitoring
				}else{
					cout << "Finished Tour" << endl;
				}
			}
		}
		else if(key=="DEPLOY"){
			string dstr = msg.GetString();
			if(dstr=="true"){
				deployed = true;
			}else{
				deployed = false;
			}
		}
		else if(key=="WPT_INDEX"){
			current_wpt = msg.GetDouble()-1;
		}
	}
	return(true);
}

void GenPath::solveTSP()
{
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
	visited_x = start_x; visited_y = start_y;
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
	publishSegList(xsol,ysol,"_tsp",vname);	//Final answer
	xvec = xsol; yvec = ysol;
	monitoring = true;
	cout << "Finished Computing" << endl;
}

void GenPath::publishSegList(vector<double> xin, vector<double> yin, string label, string vname){
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

	if(vname=="HENRY"){
		seglist_label="edge_color=white,vertex_color=blue,vertex_size=10,edge_size=1";
	}else{
		seglist_label="edge_color=pink,vertex_color=red,vertex_size=10,edge_size=1";
	}

	string desc = "label="+vname+label+","+seglist_label;
	pointsOut << ","<<desc;
	m_Comms.Notify("VIEW_SEGLIST",pointsOut.str());
	m_Comms.Notify("TSP_UPDATES",waypointsOut.str());
}

double GenPath::computeTSPDist(vector<double> xin, vector<double> yin){
	double total_distance=0;
	for(int i=1;i<xin.size();i++){
		total_distance += sqrt(pow(xin[i-1]-xin[i],2)+pow(yin[i-1]-yin[i],2));
	}
	return total_distance;
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool GenPath::OnConnectToServer()
{
	return true;
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool GenPath::Iterate()
{
	return true;
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool GenPath::OnStartUp()
{
	m_Comms.Register("VISIT_POINT", 0);
	m_Comms.Register("NAV_X",0);
	m_Comms.Register("NAV_Y",0);
	m_Comms.Register("GENPATH_REGENERATE",0);
	m_Comms.Register("DEPLOY",0);
	m_Comms.Register("WPT_INDEX",0);

	string start_pos;
	m_MissionReader.GetConfigurationParam("start_pos", start_pos);
	start_x = boost::lexical_cast<double>(MOOSChomp(start_pos,","));
	start_y = boost::lexical_cast<double>(start_pos);

	string radius;
	m_MissionReader.GetConfigurationParam("visit_radius", radius);
	visit_radius = boost::lexical_cast<double>(radius);

	m_MissionReader.GetConfigurationParam("vname", vname);

	m_Comms.Notify("TIMER_PAUSE","false");
	return true;
}
