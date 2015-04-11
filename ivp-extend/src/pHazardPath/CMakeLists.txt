# ---------------------------------------------
# pHazardPath
#        File: CMakeLists.txt
#  Created on: Apr 10, 2015
#      Author: Mei Cheung
# ---------------------------------------------

# FILE(GLOB SRC *.cpp)

SET(SRC
    HazardPath.cpp
    HazardPath_Info.cpp
    main.cpp
)



ADD_EXECUTABLE(pHazardPath ${SRC})

TARGET_LINK_LIBRARIES(pHazardPath
    mbutil
    ${MOOS_LIBRARIES}
    ${MOOS_DEPEND_LIBRARIES}
    )

