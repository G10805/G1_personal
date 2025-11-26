# User Variables
set (SUPPORT_FUNCTION_CALL_TRACE 1)

# Custom additions (begin)

#set(CAMX_PRODUCT, "msmnile")
execute_process(
        COMMAND
            python2 ${CAMX_CDK_PATH}/topology/usecasecomposer.py -i ${CAMX_VENDOR_PATH}/topology/qcom/msmnile/msmnile_usecase.xml -o ${CAMX_VENDOR_PATH}/topology/qcom/msmnile/g_msmnile_usecase.xml -d ${CAMX_VENDOR_PATH}/topology/qcom/usecase-components -t msmnile -H ${CAMX_CDK_PATH}/topology/xmlheader.txt
)

add_custom_command (
        OUTPUT
               ${CAMX_VENDOR_PATH}/chioverride/default/g_pipelines.h
        DEPENDS
               ${CAMX_VENDOR_PATH}/topology/qcom/common/common_usecase.xml
               ${CAMX_VENDOR_PATH}/topology/qcom/msmnile/msmnile_usecase.xml
        COMMAND pwd
        COMMAND perl
                ${CAMX_CDK_PATH}/topology/usecaseconverter.pl
                ${CAMX_VENDOR_PATH}/topology/qcom/common/common_usecase.xml
                ${CAMX_VENDOR_PATH}/topology/qcom/msmnile/g_msmnile_usecase.xml
                ${CAMX_VENDOR_PATH}/chioverride/default/g_pipelines.h
        COMMENT "Generate Usecases"
        )

add_custom_target (autogenUsecases
        DEPENDS
                ${CAMX_VENDOR_PATH}/chioverride/default/g_pipelines.h
        )

# Common Include
include (${CAMX_CDK_PATH}/../linuxembedded/common.cmake)

# Files and Build Type
set(SOURCE_FILES
    ../../chxadvancedcamerausecase.cpp
    ../../chxextensioninterface.cpp
    ../../chxextensionmodule.cpp
    ../../chxfeature.cpp
    ../../chxfeaturehdr.cpp
    ../../chxfeaturemultiframe.cpp
    ../../chxfeaturemfnr.cpp
    ../../chxfeaturemfsr.cpp
    ../../chxfeaturequadcfa.cpp
    ../../chxfeaturerawjpeg.cpp
    ../../chxfeatureyuvcb.cpp
    ../../chxfeaturezsl.cpp
    ../../chxmetadata.cpp
    ../../chxmulticamcontroller.cpp
    ../../chxpipeline.cpp
    ../../chxsensorselectmode.cpp
    ../../chxsession.cpp
    ../../chxusecase.cpp
    ../../chxusecasedefault.cpp
    ../../chxusecasemc.cpp
    ../../chxusecasevrmc.cpp
    ../../chxusecasequadcfa.cpp
   # ../../chxusecasesuperslowmotionfrc.cpp
    ../../chxusecasetorch.cpp
    ../../chxusecaseutils.cpp
    ../../chxutils.cpp
    ../../chxperf.cpp
    ../../chxzoomtranslator.cpp
)

if(${ANDROID_API} EQUAL 28)
    add_library (com.qti.chi.override SHARED ${SOURCE_FILES} ../../chxusecasedual.cpp)
else()
    add_library (com.qti.chi.override SHARED ${SOURCE_FILES})
endif()



# Libraries to compile
target_link_libraries (com.qti.chi.override dl)
target_link_libraries (com.qti.chi.override cutils)
target_link_libraries (com.qti.chi.override log)
target_link_libraries (com.qti.chi.override sync)
target_link_libraries (com.qti.chi.override gbm)
target_link_libraries (com.qti.chi.override camera_metadata)

# Include Paths
include_directories (${CAMX_C_INCLUDES})
include_directories (../..)
include_directories (${CAMX_CDK_PATH}/utils)
include_directories (${CAMX_CDK_PATH}/common)
#include_directories (../../../../../../camx/build/infrastructure/makemeister/../../../../chi-cdk/vendor/chioverride/default/build/android/hardware/libhardware/include/hardware)
#include_directories (../../../../../../camx/build/infrastructure/makemeister/../../../../chi-cdk/vendor/chioverride/default/build/android/hardware/qcom/display/libgralloc)
#include_directories (../../../../../../camx/build/infrastructure/makemeister/../../../../chi-cdk/vendor/chioverride/default/build/android/system/core/libsync/include)
#include_directories (../../../../../../camx/build/infrastructure/makemeister/../../../../chi-cdk/vendor/chioverride/default/build/android/system/core/include)
#include_directories (../../../../../../camx/build/infrastructure/makemeister/../../../../chi-cdk/vendor/chioverride/default/build/android/system/media/camera/include)
#include_directories (../../../../../../camx/build/infrastructure/makemeister/../../../../chi-cdk/vendor/chioverride/default/build/android/system/media/private/camera/include)

# Dependencies
add_dependencies (com.qti.chi.override autogenUsecases)

# Definitions
add_definitions (${CAMX_CFLAGS})
add_definitions (${CAMX_CPPFLAGS})
# NO DATA FOR SECTION: Custom additions (end)

# Auto-Gen
#add_dependencies(com.qti.chi.override autogen)

install(TARGETS com.qti.chi.override LIBRARY DESTINATION ${CAMX_INSTALL_LIBDIR}/hw)

if (CONFIG STREQUAL "Debug")
    add_definitions( -g )
endif()

