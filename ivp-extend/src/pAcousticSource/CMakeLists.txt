#--------------------------------------------------------
# The CMakeLists.txt for:                       pAcousticSource
# Author(s):                              
#--------------------------------------------------------

SET(SRC
  AcousticSource.cpp
  AcousticSource_Info.cpp
  main.cpp
)

ADD_EXECUTABLE(pAcousticSource ${SRC})

TARGET_LINK_LIBRARIES(pAcousticSource
   ${MOOS_LIBRARIES}
   mbutil
   m
   pthread)

