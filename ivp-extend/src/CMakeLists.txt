##############################################################################
# FILE:        moos-ivp-extend/src/CMakeLists.txt
# DATE:        2010/09/07
# DESCRIPTION: CMakeLists.txt file for the moos-ivp-extend source directory
##############################################################################


#============================================================================
# Add the libraries in the current directory to the include path
#============================================================================
FILE(GLOB LOCAL_LIBRARY_DIRS ./lib_*)
INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS}
                    "./protobuf"
                    "./lib_henrik_util")

#============================================================================
# List the subdirectories to build...
#============================================================================

#ADD_SUBDIRECTORY(protobuf)
#ADD_SUBDIRECTORY(protobufExample)
#ADD_SUBDIRECTORY(pPrimeFactor)
#ADD_SUBDIRECTORY(pOdometry)
#ADD_SUBDIRECTORY(pPrimeFactorTester)
#ADD_SUBDIRECTORY(pPointAssign)
#ADD_SUBDIRECTORY(pGenPath)
#ADD_SUBDIRECTORY(uFldHazardMgrX)
#ADD_SUBDIRECTORY(pFrontEstimate)
#ADD_SUBDIRECTORY(lib_henrik_util)
ADD_SUBDIRECTORY(pPeriodicDrive) 
ADD_SUBDIRECTORY(pAcousticSource) 

#ADD_SUBDIRECTORY(lab_05)
#ADD_SUBDIRECTORY(lab_12)

#IF(BUILD_SHORE_APPS)
    # optionally build shoreside only apps here

#    ADD_SUBDIRECTORY(simple_acomms_parser)
#ENDIF(BUILD_SHORE_APPS)

##############################################################################
#                           END of CMakeLists.txt
##############################################################################

