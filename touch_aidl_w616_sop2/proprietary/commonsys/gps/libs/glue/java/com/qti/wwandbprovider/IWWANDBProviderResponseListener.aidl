/* ======================================================================
*  Copyright (c) 2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.wwandbprovider;

import com.qti.wwandbreceiver.BSInfo;
import java.util.List;
import android.location.Location;
import com.qti.wwandbprovider.BSObsLocationData;

oneway interface IWWANDBProviderResponseListener {

    void onBsObsLocDataAvailable(in List<BSObsLocationData> bsObsList, in int bsStatus);

    void onServiceRequest();
}
