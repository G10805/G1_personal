/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlNtripConnectionParams {
    String hostNameOrIp;
    String mountPoint;
    String username;
    String password;
    int port;
    boolean requiresNmeaLocation;
    boolean useSSL;
}

