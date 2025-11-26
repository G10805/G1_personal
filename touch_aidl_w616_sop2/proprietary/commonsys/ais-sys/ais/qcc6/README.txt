The project ais-sys contains system based libs and test apps. These were originally part of the ais folder - component auto-camx.lnx.2.0*.
This project split was done to have separate components/projects for system libs and vendor libs, in order to support Split SI model.
Except evs_ais_app and qcarcam HAL client, remaining source files under ais-sys are expected to be same as the ones in ais/.
Ensure that the following folders/files under path vendor/qcom/proprietary/commonsys/ais-sys and vendor/qcom/proprietary/ais are identical:
 - API
 - CameraOSServices
 - Common/inc
 - Common/src
 - test/qcarcam_hidl_test/src/* and test/qcarcam_test/src/*
 - test/test_util/inc
 - test/test_util/src/test_util*.cpp
 - test/test_util/src/la/test_util_la*
