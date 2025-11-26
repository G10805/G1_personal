# Top-level CAMX driver's CMakeLists.txt

cmake_minimum_required(VERSION 2.6)

project (chi-cdk)

set (CAMX_OUT_PATH ${PROJECT_BINARY_DIR})
set (CAMX_CDK_PATH ${CMAKE_CURRENT_LIST_DIR}/../cdk)
set (CAMX_VENDOR_PATH ${CMAKE_CURRENT_LIST_DIR}/../vendor)

set (ANDROID_API 28)

if(${ANDROID_API} EQUAL 28)
add_definitions (-DCAMX_ANDROID_API=28)
else()
add_definitions (-DCAMX_ANDROID_API=27)
endif()

execute_process(
      COMMAND
        python2 ${CAMX_CDK_PATH}/tools/buildbins.py
)

set (CAMX_LIB_OUTPUT_PATH camera/components)
# settings and SCOPE generation
# If adding outputs, add to custom_target autogen as well

include (${CMAKE_CURRENT_LIST_DIR}/common.cmake)

add_subdirectory (${CAMX_VENDOR_PATH}/chioverride/default/build/linuxembedded ${CAMX_OUT_PATH}/vendor)
add_subdirectory (${CAMX_VENDOR_PATH}/node/awbwrapper/build/linuxembedded ${CAMX_OUT_PATH}/awbwrapper)
#add_subdirectory (${CAMX_VENDOR_PATH}/node/chiutils/build/linuxembedded ${CAMX_OUT_PATH}/vendor/chiutils)
add_subdirectory (${CAMX_VENDOR_PATH}/node/dummyrtb/build/linuxembedded ${CAMX_OUT_PATH}/vendor/dummyrtb)
add_subdirectory (${CAMX_VENDOR_PATH}/node/dummysat/build/linuxembedded ${CAMX_OUT_PATH}/vendor/dummysat)
add_subdirectory (${CAMX_VENDOR_PATH}/node/dummystich/build/linuxembedded ${CAMX_OUT_PATH}/vendor/dummystitch)
add_subdirectory (${CAMX_VENDOR_PATH}/node/hafoverride/build/linuxembedded ${CAMX_OUT_PATH}/vendor/hafoverride)
add_subdirectory (${CAMX_VENDOR_PATH}/node/hvx/addconstant/build/linuxembedded ${CAMX_OUT_PATH}/hvx/addconstant)
add_subdirectory (${CAMX_VENDOR_PATH}/node/hvx/binning/build/linuxembedded ${CAMX_OUT_PATH}/hvx/binning)
add_subdirectory (${CAMX_VENDOR_PATH}/node/gpu/build/linuxembedded ${CAMX_OUT_PATH}/vendor/gpu)
add_subdirectory (${CAMX_VENDOR_PATH}/node/memcpy/build/linuxembedded ${CAMX_OUT_PATH}/vendor/memcpy)
add_subdirectory (${CAMX_VENDOR_PATH}/node/staticaecalgo/build/linuxembedded ${CAMX_OUT_PATH}/staticaecalgo)
add_subdirectory (${CAMX_VENDOR_PATH}/node/staticafalgo/build/linuxembedded ${CAMX_OUT_PATH}/staticafalgo)
add_subdirectory (${CAMX_VENDOR_PATH}/node/staticawbalgo/build/linuxembedded ${CAMX_OUT_PATH}/staticawbalgo)
add_subdirectory (${CAMX_VENDOR_PATH}/node/staticpdlibalgo/build/linuxembedded ${CAMX_OUT_PATH}/staticpdlibalgo)
add_subdirectory (${CAMX_VENDOR_PATH}/node/remosaic/build/linuxembedded ${CAMX_OUT_PATH}/vendor/remosaic)
add_subdirectory (${CAMX_VENDOR_PATH}/sensor/default/max9296_ar0231/build/linuxembedded ${CAMX_OUT_PATH}/sensor/max9296_ar0231)
add_subdirectory (${CAMX_VENDOR_PATH}/sensor/default/max9296_ar0231rt/build/linuxembedded ${CAMX_OUT_PATH}/sensor/max9296_ar0231rt)
add_subdirectory (${CAMX_VENDOR_PATH}/sensor/default/ti960_ox03a10/build/linuxembedded ${CAMX_OUT_PATH}/sensor/ti960_ox03a10)
add_subdirectory (${CAMX_VENDOR_PATH}/sensor/default/max9296_ar0820/build/linuxembedded ${CAMX_OUT_PATH}/sensor/max9296_ar0820)
add_subdirectory (${CAMX_VENDOR_PATH}/sensor/default/max9296_ar0820rt/build/linuxembedded ${CAMX_OUT_PATH}/sensor/max9296_ar0820rt)
#add_subdirectory (${CAMX_VENDOR_PATH}/sensor/default/max9296_ar0231isp/build/linuxembedded ${CAMX_OUT_PATH}/sensor/max9296_ar0231isp)
#add_subdirectory (${CAMX_VENDOR_PATH}/sensor/default/test/build/linuxembedded ${CAMX_OUT_PATH}/sensor/test)
add_subdirectory (${CAMX_VENDOR_PATH}/node/shdr/build/cmake ${CAMX_OUT_PATH}/vendor/shdr)

#add_subdirectory (${CAMX_PATH}/src/chiiqutils/build/linuxembedded ${CAMX_OUT_PATH}/chiiqutils)
