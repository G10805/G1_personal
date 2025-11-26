/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
package com.qualcomm.qti.locationqesdkdemo.tp1;

import com.qualcomm.qti.qesdk.Location.PP_eDGNSSEnums;

import java.io.Serializable;

public class ApiInfo implements Serializable {

    public static final String API_IQESDK_INIT = "IQesdk . init";

    public static final String API_PP_EDGNSS_REGISTER_LOCATION_CAPABILITIES_CB =
            "PP_eDGNSSManager . registerLocationCapabilitiesCallback";
    public static final String API_PP_EDGNSS_REQUEST_PRECISE_LOCATION_UPDATES =
            "PP_eDGNSSManager . requestPreciseLocationUpdates";
    public static final String API_PP_EDGNSS_REMOVE_PRECISE_LOCATION_UPDATES =
            "PP_eDGNSSManager . removePreciseLocationUpdates";
    public static final String API_PP_EDGNSS_UPDATE_NTRIP_GGA_CONSENT =
            "PP_eDGNSSManager . updateNTRIPGGAConsent";
    public static final String API_PP_EDGNSS_ENABLE_PPE_NTRIP_STREAM =
            "PP_eDGNSSManager . enablePPENtripStream";
    public static final String API_PP_EDGNSS_DISABLE_PPE_NTRIP_STREAM =
            "PP_eDGNSSManager . disablePPENtripStream";
    public static final String API_PP_EDGNSS_REGISTER_AS_CORRECTION_DATA_SOURCE =
            "PP_eDGNSSManager . registerAsCorrectionDataSource";
    public static final String API_PP_EDGNSS_DEREGISTER_AS_CORRECTION_DATA_SOURCE =
            "PP_eDGNSSManager . deRegisterAsCorrectionDataSource";
    public static final String API_PP_EDGNSS_INJECT_CORRECTION_DATA =
            "PP_eDGNSSManager . injectCorrectionData";

    public static final String API_PP_RTK_REQUEST_PRECISE_LOCATION_UPDATES =
            "PP_RTKManager . requestPreciseLocationUpdates";
    public static final String API_PP_RTK_REMOVE_PRECISE_LOCATION_UPDATES =
            "PP_RTKManager . removePreciseLocationUpdates";

    public String apiName;

    // API category helper
    public static boolean isEdgnssApi(String apiName) {
        switch(apiName) {
            case API_PP_EDGNSS_REGISTER_LOCATION_CAPABILITIES_CB:
            case API_PP_EDGNSS_REQUEST_PRECISE_LOCATION_UPDATES:
            case API_PP_EDGNSS_REMOVE_PRECISE_LOCATION_UPDATES:
            case API_PP_EDGNSS_ENABLE_PPE_NTRIP_STREAM:
            case API_PP_EDGNSS_DISABLE_PPE_NTRIP_STREAM:
            case API_PP_EDGNSS_REGISTER_AS_CORRECTION_DATA_SOURCE:
            case API_PP_EDGNSS_DEREGISTER_AS_CORRECTION_DATA_SOURCE:
            case API_PP_EDGNSS_UPDATE_NTRIP_GGA_CONSENT:
            case API_PP_EDGNSS_INJECT_CORRECTION_DATA:
                return true;
            default:
                return false;
        }
    }
    public static boolean isRtkApi(String apiName) {
        switch(apiName) {
            case API_PP_RTK_REQUEST_PRECISE_LOCATION_UPDATES:
            case API_PP_RTK_REMOVE_PRECISE_LOCATION_UPDATES:
                return true;
            default:
                return false;
        }
    }

    // Arguments, populated as per mApiName

    // requestPreciseLocationUpdates
    public int minIntervalMillis;

    // updateNTRIPGGAConsent
    public boolean ntripGGAConsentAccepted;

    // enablePPENtripStream
    String hostNameOrIp;
    String mountPoint;
    String username;
    String password;
    int port;
    boolean requiresNmeaLocation;
    boolean useSSL;
    boolean enableRTKEngine;

    // registerAsCorrectionDataSource
    PP_eDGNSSEnums.CorrectionDataType correctionDataType;

    // injectCorrectionData
    boolean appNtripClientEnabled;
    boolean appNtripUseNtrip2Version;
    String appNtripHostName;
    int appNtripPort;
    String appNtripMountPoint;
    String appNtripUsernamePwdEncodedInBase64Format;

    // Not directly triggered by the user
    byte[] correctionData;
}
