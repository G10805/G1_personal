/* ======================================================================
*  Copyright (c) 2022 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.debugreport;
import com.qti.debugreport.IZatXTRAStatus;

oneway interface IXtraStatusCallback {
    void onXtraStatusChanged(in IZatXTRAStatus status);
}
