/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
package my.tests.snapdragonsdktest;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.widget.Button;
import com.qti.location.sdk.IZatGnssConfigServices;
import com.qti.location.sdk.IZatManager;

public class GtpOemConsentActivity extends Activity {

    private static final String TAG = "GTPTest";

    IZatGnssConfigServices mGnssService;

    @Override
    protected void onCreate(Bundle savedInstanceState ) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_oem_consent);

        try {
            IZatManager mIzatMgr = IZatManager.getInstance(getApplicationContext());

            mGnssService = mIzatMgr.connectGnssConfigServices();
        } catch (SecurityException e) {
            return;
        }

        if (null == mGnssService) {
            Log.d(TAG, "Failed to get Gnss Config Service");
            return;
        }

        final Button setConsentTrueButton = findViewById(R.id.btnOemConsentTrue);
        final Button setConsentFalseButton = findViewById(R.id.btnOemConsentFalse);

        setConsentTrueButton.setOnClickListener(v -> {
            Log.d(TAG, "Set OEM User Consent True");
            mGnssService.setNetworkLocationUserConsent(true);
        });

        setConsentFalseButton.setOnClickListener(v -> {
            Log.d(TAG, "Set OEM User Consent False");
            mGnssService.setNetworkLocationUserConsent(false);
        });
    }
}
