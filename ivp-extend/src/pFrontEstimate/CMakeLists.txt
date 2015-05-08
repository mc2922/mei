# ---------------------------------------------
# pFrontEstimate
#        File: CMakeLists.txt
#  Created on: May 04, 2015
#      Author: mc2922
# ---------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
    FrontEstimate.cpp
    FrontEstimate_Info.cpp
    main.cpp
)



ADD_EXECUTABLE(pFrontEstimate ${SRC})

TARGET_LINK_LIBRARIES(pFrontEstimate
    mbutil
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
	henrik_util
	HoverAcomms
	boost_system
	mei_proto
   	goby_acomms	
    )

