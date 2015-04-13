/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: HazardMgr.h                                          */
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

#ifndef UFLD_HAZARD_MGR_HEADER
#define UFLD_HAZARD_MGR_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include <vector>
#include <boost/lexical_cast.hpp>
#include "Hazard.pb.h"
#include "goby/acomms/dccl.h"

class HazardMgrX : public AppCastingMOOSApp
{
 public:
   HazardMgrX();
   ~HazardMgrX() {};

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected: // Registration, Configuration, Mail handling utils
   bool handleMailSensorConfigAck(std::string);
   bool handleMailSensorOptionsSummary(std::string) {return(true);};
   void handleMailMissionParams(std::string);

 protected: 
   void postSensorConfigRequest();
   void postSensorInfoRequest();
   std::string getHazardSetReport();
   void publishSegList(std::vector<double> xin, std::vector<double> yin);
   void segmentSpace();
   void solveTSP();
   double computeTSPDist(std::vector<double> xin, std::vector<double> yin);
   void switchType();

 private: // Configuration variables
   double      m_swath_width_desired;
   double      m_pd_desired;
   std::string m_report_name;

 private: // State variables
   bool   m_sensor_config_requested;
   bool   m_sensor_config_set;

   unsigned int m_sensor_config_reqs;
   unsigned int m_sensor_config_acks;

   unsigned int m_sensor_report_reqs;
   unsigned int m_detection_reports;
   unsigned int m_summary_reports;

   double m_swath_width_granted;
   double m_pd_granted;

   std::vector<double> xset,yset;
   std::vector<int> labelset;
   std::vector<std::string> typeset;
   std::vector<int> dset,cset;

   int id;
   int myeast_it,mywest_it;
   int coopeast_it,coopwest_it;
   double myx,myy;
   double xmin,xmax,ymin,ymax;
   double ym;
   std::string type;
   double skew_coop,skew;
   double start_time,max_time;
   double time_counter;
   int cycle_counter;

   goby::acomms::DCCLCodec* codec;
   HazardList hlist;

   std::string state;
};

#endif 




