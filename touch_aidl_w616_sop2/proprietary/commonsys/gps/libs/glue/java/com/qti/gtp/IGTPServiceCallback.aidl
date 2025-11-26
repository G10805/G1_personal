/* ======================================================================
*  Copyright (c) 2022 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/
package com.qti.gtp;

import android.location.Location;

oneway interface IGTPServiceCallback {
    void onLocationAvailable(in Location location);
}
