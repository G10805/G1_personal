/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
parcelable LocAidlAddress {
    String adminArea;
    String countryCode;
    String countryName;
    String featureName;
    boolean hasLatitude;
    double latitude;
    boolean hasLongitude;
    double longitude;
    String locale;
    String locality;
    String phone;
    String postalCode;
    String premises;
    String subAdminArea;
    String subLocality;
    String thoroughfare;
    String subThoroughfare;
    String url;
}
