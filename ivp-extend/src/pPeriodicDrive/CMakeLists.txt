#--------------------------------------------------------
# The CMakeLists.txt for:                       pPeriodicDrive
# Author(s):                              
#--------------------------------------------------------

SET(SRC
  PeriodicDrive.cpp
  PeriodicDrive_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pPeriodicDrive ${SRC})

TARGET_LINK_LIBRARIES(pPeriodicDrive
   ${MOOS_LIBRARIES}
   mbutil
   m
   pthread)

