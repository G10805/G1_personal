/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

@VintfStability
interface ILocAidlRilInfoMonitor {
    void chargerStatusInject(in int status);

    void cinfoInject(in int cid, in int lac, in int mnc, in int mcc, in boolean roaming);

    void init();

    void niSuplInit(in String str);

    void oosInform();
}
