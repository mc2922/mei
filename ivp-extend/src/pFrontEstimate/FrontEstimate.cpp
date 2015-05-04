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
	min_alpha = 500;
	max_alpha = 500;
	min_beta = 10;
	max_beta = 50;
	min_T_N = 15;
	max_T_N = 25;
	min_T_S = 20;
	max_T_S = 30;

	id = 1;
	start = "false";
	state = "start";
	navx = 0; navy = 0;
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
	}

	return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool FrontEstimate::Iterate()
{
	if(start=="true"){
		if(id==1){
			if(state=="start"){  //Estimate North

			}
		}

		if(id==2){
			if(state=="start"){  //Estimate South

			}
		}
	}
	return true;
}

bool FrontEstimate::OnStartUp()
{
	string config_val;
	m_MissionReader.GetConfigurationParam("id", config_val);
	id = boost::lexical_cast<double>(config_val);

	//Front estimate parameters
	m_MissionReader.GetConfigurationParam("min_offset", config_val);
	min_offset = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("max_offset", config_val);
	max_offset = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("min_angle", config_val);
	min_angle = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("max_angle", config_val);
	max_angle = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("min_amplitude", config_val);
	min_amplitude = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("max_amplitude", config_val);
	max_amplitude = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("min_period", config_val);
	min_period = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("max_period", config_val);
	max_period = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("min_wavelength", config_val);
	min_wavelength = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("max_wavelength", config_val);
	max_wavelength = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("min_alpha", config_val);
	min_alpha = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("max_alpha", config_val);
	max_alpha = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("min_beta", config_val);
	min_beta = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("max_beta", config_val);
	max_beta = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("min_T_N", config_val);
	min_T_N = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("max_T_N", config_val);
	max_T_N = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("min_T_S", config_val);
	min_T_S = boost::lexical_cast<double>(config_val);
	m_MissionReader.GetConfigurationParam("max_T_S", config_val);
	max_T_S = boost::lexical_cast<double>(config_val);

	return true;
}

bool FrontEstimate::OnConnectToServer()
{
	m_Comms.Register("FRONT_ESTIMATE_START",0);
	m_Comms.Register("NAV_X", 0);
	m_Comms.Register("NAV_Y", 0);
	return true;
}
