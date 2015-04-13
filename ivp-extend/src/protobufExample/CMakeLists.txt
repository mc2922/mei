# ---------------------------------------------
# protobufExample, protobufDCCLExample
#        File: CMakeLists.txt
#  Created on: Sep 10, 2013
#      Author: Josh Leighton
# ---------------------------------------------

add_executable(protobufExample protobufExample.cpp)
add_executable(protobufDCCLExample protobufDCCLExample.cpp)

target_link_libraries(protobufExample
    mei_proto
    )
    
target_link_libraries(protobufDCCLExample
    mei_proto
    goby_acomms
    protobuf
    boost_system
    )
