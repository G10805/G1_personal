project(AIS)
cmake_minimum_required(VERSION 2.6)

set (CHROMATIX_VERSION 0309)
set (AIS_BUILD_WITH_CAMX "true")
set (AIS_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set (CHI_CDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../chi-cdk)

if(AIS_DATADIR_ENABLE)
add_definitions (-DAIS_DATADIR_ENABLE)
endif(AIS_DATADIR_ENABLE)

if ("$ENV{AIS_MACHINE_TYPE}" STREQUAL "HYP")
MESSAGE( STATUS "Hypervisor  build")

add_subdirectory(libais_log)
add_subdirectory(libais_client)
add_subdirectory(libtest_util)
add_subdirectory(qcarcam_test)

if ("$ENV{AIS_V4L2_DISABLE}" STREQUAL "true")
else ("$ENV{AIS_V4L2_DISABLE}" STREQUAL "true")
add_subdirectory(ais_v4l2_proxy)
add_subdirectory(qcarcam_v4l2_test)
endif ("$ENV{AIS_V4L2_DISABLE}" STREQUAL "true")

else ("$ENV{AIS_MACHINE_TYPE}" STREQUAL "METAL")
MESSAGE( STATUS "Metal build")

if ("$ENV{AIS_EARLYSERVICE}" STREQUAL "true")
add_definitions (-DAIS_EARLYSERVICE)
endif ("$ENV{AIS_EARLYSERVICE}" STREQUAL "true")

add_subdirectory(libais)
add_subdirectory(ais_server)
add_subdirectory(libais_client)
add_subdirectory(libais_log)
add_subdirectory(libais_config)
add_subdirectory(libais_max9296)
add_subdirectory(libais_max9296b)
add_subdirectory(libais_max96712)
add_subdirectory(libais_tids90ub)
add_subdirectory(libais_sensorstub)
add_subdirectory(libtest_util)
add_subdirectory(qcarcam_test)
add_subdirectory(qcarcam_rvc)
add_subdirectory(ccidbgr)

endif ("$ENV{AIS_MACHINE_TYPE}" STREQUAL "HYP")
