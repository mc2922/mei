#--------------------------------------------------------
# The CMakeLists.txt for:                       pPeriodicDriver
# Author(s):                              
#--------------------------------------------------------

SET(SRC
  PeriodicDriver.cpp
  PeriodicDriver_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pPeriodicDriver ${SRC})

TARGET_LINK_LIBRARIES(pPeriodicDriver
   ${MOOS_LIBRARIES}
   apputil
   mbutil
   m
   pthread
   boost_system
   boost_filesystem
   boost_thread)

