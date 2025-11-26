/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
package my.tests.snapdragonsdktest;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Switch;
import android.view.View;
import android.widget.Spinner;
import android.widget.AdapterView;
import com.qti.location.sdk.IZatGtpService;
import com.qti.location.sdk.IZatGtpService.IzatGtpAccuracy;
import com.qti.location.sdk.IZatManager;
import android.os.Handler;
import android.os.Message;
import android.os.Bundle;
import android.app.AppOpsManager;
import android.location.Location;
import android.content.pm.PackageManager;
import android.content.pm.ApplicationInfo;

import java.util.ArrayList;

public class GtpActivity extends Activity {
    private static final String TAG = "GTPTest";
    private static final String NLP_TYPE_KEY = "networkLocationType";

    IZatGtpService mGtpService;
    ArrayList<String> mListOfFixes = new ArrayList<>();
    ArrayAdapter<String> mResultsAdapter;
    private SharedPreferences savedData;
    private IzatGtpAccuracy mReqAcc = IzatGtpAccuracy.NOMINAL;
    private AppOpsManager mAppOpsMgr;

    private Handler mHandler;
    private static final int MSG_UPDATE_UI = 0;
    private static final int MSG_UPDATE_LOC_REPORT = 1;
    private ListView mListView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate, this " + this);
        super.onCreate(savedInstanceState);
        Context ctx = getApplicationContext();

        setContentView(R.layout.activity_gtp);

        mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case MSG_UPDATE_LOC_REPORT:
                    {
                        Log.d(TAG, "MSG_UPDATE_LOC_REPORT");
                        Location loc = (Location)msg.obj;
                        int fixCounter = msg.arg1;
                        Bundle extra = loc.getExtras();
                        String sessionName = "";
                        String locType = "";
                        if (extra != null) {
                            locType = extra.getString(NLP_TYPE_KEY);
                            sessionName = extra.getString("sessionName");
                        }
                        mResultsAdapter.insert(sessionName + " fix " + fixCounter +
                                ": lat: " + loc.getLatitude() +
                                ", long: " + loc.getLongitude() +
                                ", Accuracy: " + loc.getAccuracy() +
                                ", NLP type: " + locType, 0);
                    }
                    case MSG_UPDATE_UI:
                    {
                        Log.d(TAG, "MSG_UPDATE_UI");
                        mResultsAdapter.notifyDataSetChanged();
                        mListView.postInvalidate();
                        break;
                    }
                    default: break;
                }
            }
        };

        mResultsAdapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, mListOfFixes);

        savedData = PreferenceManager.getDefaultSharedPreferences(ctx);
        boolean consentState = savedData.getBoolean("gtpConsent", false);

        mResultsAdapter.insert("User consent is currently set to " + consentState, 0);

        try {
            Log.d(TAG, "Get instance of Izatmanager");
            IZatManager mIzatMgr = IZatManager.getInstance(ctx);

            Log.d(TAG, "connect to GTP service");
            mGtpService = mIzatMgr.connectToGtpService();
        } catch (SecurityException e) {
            return;
        }

        if (null == mGtpService) {
            Log.d(TAG, "Failed to get GTP Service");
            return;
        }
        mResultsAdapter.insert("Connected to GTP Service", 0);
        mAppOpsMgr = getSystemService(AppOpsManager.class);
        PackageManager pm = getPackageManager();
        ApplicationInfo ai = null;
        try {
            ai = pm.getApplicationInfo("my.tests.snapdragonsdktest", PackageManager.GET_META_DATA);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        int snapUid = (ai == null? 0: ai.uid);
        Log.d(TAG, "snapUid: " + snapUid);


        final Button setConsentTrueButton = findViewById(R.id.btnConsentTrue);
        final Button setConsentFalseButton = findViewById(R.id.btnConsentFalse);
        final Button setUidModeAllowedButton = findViewById(R.id.btnModeAllowed);
        final Button setUidModeIgnoredButton = findViewById(R.id.btnModeIgnored);
        final Button setUidFineModeAllowedButton = findViewById(R.id.btnFineModeAllowed);
        final Button setUidFineModeIgnoredButton = findViewById(R.id.btnFineModeIgnored);

        final Button startActiveButton = findViewById(R.id.btnStartActiveSession);
        final Button stopActiveButton = findViewById(R.id.btnStopActiveSession);
        final EditText tbf = findViewById(R.id.timeBetweenFixes);

        final Switch passiveSwitch = findViewById(R.id.sldPassiveSession);

        mListView = findViewById(R.id.lstGTPLog);
        mListView.setAdapter(mResultsAdapter);
        Spinner spinner = (Spinner) findViewById(R.id.AccuracySpiner);
        // Create an ArrayAdapter using the string array and a default spinner layout
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
                R.array.gtp_acc_array, android.R.layout.simple_spinner_item);
        // Specify the layout to use when the list of choices appears
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        // Apply the adapter to the spinner
        spinner.setAdapter(adapter);
        spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
                    switch (pos) {
                        case 0:
                        {
                            mReqAcc = IzatGtpAccuracy.NOMINAL;
                            break;
                        }
                        case 1:
                        {
                            mReqAcc = IzatGtpAccuracy.HIGH;
                            break;
                        }
                        default:
                    }
                }

                @Override
                public void onNothingSelected(AdapterView<?> parent) {
                    mReqAcc = IzatGtpAccuracy.NOMINAL;
                }
        });

        setConsentTrueButton.setOnClickListener(v -> {
            Log.d(TAG, "Set User Consent True");
            mResultsAdapter.insert("Set User Consent True", 0);
            mHandler.obtainMessage(MSG_UPDATE_UI).sendToTarget();
            mGtpService.setUserConsent(true);
            SharedPreferences.Editor editPrefs = savedData.edit();
            editPrefs.putBoolean("gtpConsent", true);
            editPrefs.apply();
        });

        setConsentFalseButton.setOnClickListener(v -> {
            Log.d(TAG, "Set User Consent False");
            mResultsAdapter.insert("Set User Consent False", 0);
            mHandler.obtainMessage(MSG_UPDATE_UI).sendToTarget();
            mGtpService.setUserConsent(false);
            SharedPreferences.Editor editPrefs = savedData.edit();
            editPrefs.putBoolean("gtpConsent", false);
            editPrefs.apply();
        });

        startActiveButton.setOnClickListener(v -> {
            int t = Integer.parseInt(tbf.getText().toString());
            Log.d(TAG, "Start network positioning with tbf: " + t);
            mResultsAdapter.insert("Start network positioning with tbf: " + t, 0);
            mHandler.obtainMessage(MSG_UPDATE_UI).sendToTarget();

            mGtpService.requestLocationUpdates(
                    GtpTestLocationCallbackImpl.getInstance("Active", mHandler),
                        t, mReqAcc);
        });

        stopActiveButton.setOnClickListener(v -> {
            Log.d(TAG, "Stop active positioning");
            mResultsAdapter.insert("STOP active positioning", 0);
            mHandler.obtainMessage(MSG_UPDATE_UI).sendToTarget();

            mGtpService.removeLocationUpdates();
        });

        passiveSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if(isChecked) {
                Log.d(TAG, "Enable passive listener");
                mResultsAdapter.insert("Enable passive listener", 0);
                mHandler.obtainMessage(MSG_UPDATE_UI).sendToTarget();

                mGtpService.requestPassiveLocationUpdates(
                        GtpTestLocationCallbackImpl.getInstance("Passive", mHandler));
            }
            else {
                Log.d(TAG, "Remove passive listener");
                mResultsAdapter.insert("Remove passive listener", 0);
                mHandler.obtainMessage(MSG_UPDATE_UI).sendToTarget();

                mGtpService.removePassiveLocationUpdates();
            }
        });
    }

    @Override
    protected void onDestroy() {
        // de-register callback needed?
        super.onDestroy();
    }
}
