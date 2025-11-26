/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
package com.qualcomm.qti.locationqesdkdemo.tp1;

import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContract;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.qualcomm.qti.locationqesdkdemo.tp1.ntrip.NtripClient;
import com.qualcomm.qti.qesdk.Location.IPP_RTKCBs;
import com.qualcomm.qti.qesdk.Location.IPP_eDGNSSCBs;
import com.qualcomm.qti.qesdk.Location.PP_RTKEnums;
import com.qualcomm.qti.qesdk.Location.PP_RTKManager;
import com.qualcomm.qti.qesdk.Location.PP_eDGNSSEnums;
import com.qualcomm.qti.qesdk.Location.PP_eDGNSSManager;
import com.qualcomm.qti.qesdk.QesdkStatusException;
import com.qualcomm.qti.qesdkIntf.IQesdk;
import com.qualcomm.qti.qesdkIntf.IQesdkEventCallBack;

import java.util.Calendar;
import java.util.Date;

public class MainActivity extends AppCompatActivity implements AdapterView.OnItemSelectedListener {

    private final String[] mApiList = {

            "Please select an API to execute",

            ApiInfo.API_IQESDK_INIT,

            ApiInfo.API_PP_EDGNSS_REGISTER_LOCATION_CAPABILITIES_CB,
            ApiInfo.API_PP_EDGNSS_REQUEST_PRECISE_LOCATION_UPDATES,
            ApiInfo.API_PP_EDGNSS_REMOVE_PRECISE_LOCATION_UPDATES,
            ApiInfo.API_PP_EDGNSS_UPDATE_NTRIP_GGA_CONSENT,
            ApiInfo.API_PP_EDGNSS_ENABLE_PPE_NTRIP_STREAM,
            ApiInfo.API_PP_EDGNSS_DISABLE_PPE_NTRIP_STREAM,
            ApiInfo.API_PP_EDGNSS_REGISTER_AS_CORRECTION_DATA_SOURCE,
            ApiInfo.API_PP_EDGNSS_DEREGISTER_AS_CORRECTION_DATA_SOURCE,
            ApiInfo.API_PP_EDGNSS_INJECT_CORRECTION_DATA,

            ApiInfo.API_PP_RTK_REQUEST_PRECISE_LOCATION_UPDATES,
            ApiInfo.API_PP_RTK_REMOVE_PRECISE_LOCATION_UPDATES
    };

    private IQesdk mQesdkManager;
    private PP_eDGNSSManager mPpeEdgnssManager;
    private PP_RTKManager mPpeRtkManager;

    private ApiInfo mApiToExecute = new ApiInfo();

    private IPP_eDGNSSCBs.ICorrectionStreamingControlCallback streamingControlCallback;
    private IPP_eDGNSSCBs.ILocationCapabilitiesCallback locationCapabilitiesCallback;
    private IPP_eDGNSSCBs.ILocationReportCallback eDgnssLocationReportCallback;
    private IPP_RTKCBs.ILocationReportCallback rtkLocationReportCallback;

    private NtripClient.CorrectionDataCallback appNtripClientCorrectionDataCb;

    private final NtripClient mNtripClient = new NtripClient((tag, msg) -> logEvent(msg, tag));

    // Arguments fetch activity
    private ActivityResultLauncher<ApiInfo> fetchArgumentsActivity;

    // Messages to be processed off main thread
    private Handler mHandler;
    private static final int MSG_LOG_EVENT = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // QESDK Callbacks
        setupCallbacks();

        // To enable scrolling
        enableScrollingInTextViews();

        // Set API list in spinner
        setupApiSpinner();

        // Setup activity to fetch API arguments
        setupFetchArgumentsActivity();

        // Execute API button click handler
        setupExecuteApiButtonClickHandler();

        // Setup handler thread to handle UI updates
        setupHandlerThread();
    }

    @Override
    public void onItemSelected(AdapterView<?> adapterView, View view, int index, long l) {

        // API for execution has been selected.
        if (index > 0) {
            prepareApiForExecution(index);
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> adapterView) {
        // no-op
    }

    private void setupHandlerThread() {

        //Create a Handler thread to handle UI update
        mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case MSG_LOG_EVENT:
                        handleMsgLogEvent(msg);
                        break;
                    default:
                }
            }
        };
    }

    private void handleMsgLogEvent(Message msg) {

        String msgText = (String)msg.obj;
        TextView eventLog = findViewById(R.id.eventLogTextView);
        Date currentTime = Calendar.getInstance().getTime();
        String text = eventLog.getText().toString();
        text = currentTime + "\n" + msgText + "\n\n" + text;
        eventLog.setText(text);
    }

    private void enableScrollingInTextViews() {

        TextView eventLog = findViewById(R.id.eventLogTextView);
        eventLog.setMovementMethod(new ScrollingMovementMethod());
        TextView selectedApiInfo = findViewById(R.id.selectedApiInfoTextView);
        selectedApiInfo.setMovementMethod(new ScrollingMovementMethod());
    }

    private void setupApiSpinner() {

        Spinner spinner = findViewById(R.id.apiToExecuteSpinner);
        spinner.setOnItemSelectedListener(this);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                this, R.layout.spinner_element, mApiList) {
            @Override
            public boolean isEnabled(int position) {
                return position != 0;
            }

            @Override
            public View getDropDownView(int position, @Nullable View convertView, @NonNull ViewGroup parent) {
                View view = super.getDropDownView(position, convertView, parent);
                TextView api = (TextView) view;
                if (position == 0) api.setTextColor(Color.GRAY);
                else api.setTextColor(Color.BLACK);
                return view;
            }
        };
        adapter.setDropDownViewResource(R.layout.spinner_element);
        spinner.setAdapter(adapter);
    }

    private void setupExecuteApiButtonClickHandler() {

        Button executeApiButton = findViewById(R.id.executeApiButton);
        executeApiButton.setOnClickListener(view -> executePreparedApi());
    }

    private void setupFetchArgumentsActivity() {

        if (fetchArgumentsActivity == null) {
            fetchArgumentsActivity = registerForActivityResult(
                    new ActivityResultContract<ApiInfo, ApiInfo>() {
                        @NonNull
                        @Override
                        public Intent createIntent(@NonNull Context context, ApiInfo input) {
                            Intent intent = new Intent(
                                    MainActivity.this, FetchArgumentsActivity.class);
                            intent.putExtra("api_info", input);
                            return intent;
                        }

                        @Override
                        public ApiInfo parseResult(int resultCode, @Nullable Intent intent) {
                            if (resultCode == RESULT_OK) {
                                assert intent != null;
                                return (ApiInfo) intent.getSerializableExtra("api_info");
                            } else {
                                logEvent("Fetch arguments failed, result: " + resultCode);
                            }
                            return null;
                        }
                    },
                    apiInfo -> {
                        if (apiInfo != null) {
                            logEvent("API Arguments received, now Execute API.");
                            mApiToExecute = apiInfo;
                            logApiInfo();
                        } else {
                            logEvent("Fetch arguments failed.");
                        }
                    });
        } else {
            logEvent("fetchArgumentsActivity already setup.");
        }
    }

    private void logEvent(String msg) {
        String TAG = "LocationQESDK.MainActivity";
        logEvent(msg, TAG);
    }
    private void logEvent(String msg, String tag) {

        Log.i(tag, msg);
        mHandler.obtainMessage(MSG_LOG_EVENT, msg).sendToTarget();
    }

    private void logApiInfo() {

        TextView apiInfo = findViewById(R.id.selectedApiInfoTextView);
        String text = "Selected API details below.\n\n" + mApiToExecute.apiName + "\n\n" +
                getApiArgumentsStringForDisplay();
        apiInfo.setText(text);
    }

    private String getApiArgumentsStringForDisplay() {

        switch (mApiToExecute.apiName) {
            case ApiInfo.API_IQESDK_INIT:
            case ApiInfo.API_PP_EDGNSS_REGISTER_LOCATION_CAPABILITIES_CB:
            case ApiInfo.API_PP_EDGNSS_REMOVE_PRECISE_LOCATION_UPDATES:
            case ApiInfo.API_PP_EDGNSS_DISABLE_PPE_NTRIP_STREAM:
            case ApiInfo.API_PP_EDGNSS_DEREGISTER_AS_CORRECTION_DATA_SOURCE:
            case ApiInfo.API_PP_RTK_REMOVE_PRECISE_LOCATION_UPDATES:
                return "No Arguments for this API";

            case ApiInfo.API_PP_EDGNSS_REQUEST_PRECISE_LOCATION_UPDATES:
            case ApiInfo.API_PP_RTK_REQUEST_PRECISE_LOCATION_UPDATES:
                return "minIntervalMillis: [" + mApiToExecute.minIntervalMillis + "]";

            case ApiInfo.API_PP_EDGNSS_UPDATE_NTRIP_GGA_CONSENT:
                return "ntripGGAConsentAccepted: [" + mApiToExecute.ntripGGAConsentAccepted + "]";

            case ApiInfo.API_PP_EDGNSS_ENABLE_PPE_NTRIP_STREAM:
                return "hostNameOrIp: [" + mApiToExecute.hostNameOrIp + "] " +
                        "mountPoint: [" + mApiToExecute.mountPoint + "] " +
                        "username: [" + mApiToExecute.username + "] " +
                        "password: [" + mApiToExecute.password + "] " +
                        "port: [" + mApiToExecute.port + "] " +
                        "requiresNmeaLocation: [" + mApiToExecute.requiresNmeaLocation + "] " +
                        "useSSL: [" + mApiToExecute.useSSL + "] " +
                        "enableRTKEngine: [" + mApiToExecute.enableRTKEngine + "]";

            case ApiInfo.API_PP_EDGNSS_REGISTER_AS_CORRECTION_DATA_SOURCE:
                return "correctionDataType: [" + mApiToExecute.correctionDataType + "]";

            case ApiInfo.API_PP_EDGNSS_INJECT_CORRECTION_DATA:
                return "appNtripClientEnabled: [" + mApiToExecute.appNtripClientEnabled + "] " +
                        "appNtripUseNtrip2Version: [" + mApiToExecute.appNtripUseNtrip2Version + "] " +
                        "appNtripHostName: [" + mApiToExecute.appNtripHostName + "] " +
                        "appNtripPort: [" + mApiToExecute.appNtripPort + "] " +
                        "appNtripMountPoint: [" + mApiToExecute.appNtripMountPoint + "] " +
                        "appNtripUsernamePwdEncodedInBase64Format: [" +
                            mApiToExecute.appNtripUsernamePwdEncodedInBase64Format + "]";

            default:
                logEvent("Invalid API Name: " + mApiToExecute.apiName);
                return "Invalid API Name";
        }
    }

    private void setupCallbacks() {

        if (locationCapabilitiesCallback == null) {
            locationCapabilitiesCallback = new IPP_eDGNSSCBs.ILocationCapabilitiesCallback() {
                @Override
                public void onValues(long l) {
                    capabilitiesCb(l);
                }
            };
        }
        if (streamingControlCallback == null) {
            streamingControlCallback = new IPP_eDGNSSCBs.ICorrectionStreamingControlCallback() {
                @Override
                public void onValues(PP_eDGNSSEnums.CorrectionStreamingState correctionStreamingState) {
                    correctionStreamingControlCb(correctionStreamingState);
                }
            };
        }
        if (eDgnssLocationReportCallback == null) {
            eDgnssLocationReportCallback = new IPP_eDGNSSCBs.ILocationReportCallback() {
                @Override
                public void onValues(
                        int locationFlags, long timestamp,
                        double latitude, double longitude, double altitude,
                        float speed, float bearing, float accuracy,
                        float verticalAccuracy, float speedAccuracy, float bearingAccuracy,
                        long elapsedRealTimeNanos, long elapsedRealTimeUncNanos,
                        PP_eDGNSSEnums.LocationQuality locationQuality) {

                    eDgnssLocationReportCb(locationFlags, timestamp, latitude, longitude, altitude,
                            speed, bearing, accuracy, verticalAccuracy, speedAccuracy,
                            bearingAccuracy, elapsedRealTimeNanos, elapsedRealTimeUncNanos,
                            locationQuality);
                }
            };
        }
        if (rtkLocationReportCallback == null) {
            rtkLocationReportCallback = new IPP_RTKCBs.ILocationReportCallback() {
                @Override
                public void onValues(
                        int locationFlags, long timestamp,
                        double latitude, double longitude, double altitude,
                        float speed, float bearing, float accuracy,
                        float verticalAccuracy, float speedAccuracy, float bearingAccuracy,
                        long elapsedRealTimeNanos, long elapsedRealTimeUncNanos,
                        PP_RTKEnums.LocationQuality locationQuality) {

                    rtkLocationReportCb(locationFlags, timestamp, latitude, longitude, altitude,
                            speed, bearing, accuracy, verticalAccuracy, speedAccuracy,
                            bearingAccuracy, elapsedRealTimeNanos, elapsedRealTimeUncNanos,
                            locationQuality);
                }
            };
        }

        // This is Application NTRIP Client Callback, not for QESDK API
        if (appNtripClientCorrectionDataCb == null) {
            appNtripClientCorrectionDataCb = new NtripClient.CorrectionDataCallback() {
                @Override
                public void injectCorrectionData(byte[] buffer) {
                    logEvent("Received correction data from App NTRIP client, length " + buffer.length);
                    if (true) {
                        mApiToExecute.correctionData = buffer;
                        try {
                            mPpeEdgnssManager.injectCorrectionData(buffer);
                        } catch (QesdkStatusException e) {
                        }

                        //executePreparedApi(ApiInfo.API_PP_EDGNSS_INJECT_CORRECTION_DATA);
                    } else {
                        logEvent("App NTRIP client disabled, unexpected callback.");
                    }
                }
            };
        }
    }

    // Fetch API arguments from user, and prepare for execution
    private void prepareApiForExecution(int apiIndex) {

        mApiToExecute.apiName = mApiList[apiIndex];

        if (mApiToExecute.apiName == null) {
            logEvent("Please select an API");
            return;
        }

        logApiInfo();

        // Fetch arguments as per API
        switch (mApiToExecute.apiName) {
            case ApiInfo.API_IQESDK_INIT:
            case ApiInfo.API_PP_EDGNSS_REGISTER_LOCATION_CAPABILITIES_CB:
            case ApiInfo.API_PP_EDGNSS_REMOVE_PRECISE_LOCATION_UPDATES:
            case ApiInfo.API_PP_EDGNSS_DISABLE_PPE_NTRIP_STREAM:
            case ApiInfo.API_PP_EDGNSS_DEREGISTER_AS_CORRECTION_DATA_SOURCE:
            case ApiInfo.API_PP_RTK_REMOVE_PRECISE_LOCATION_UPDATES:
                // no args for these APIs
                break;

            case ApiInfo.API_PP_EDGNSS_REQUEST_PRECISE_LOCATION_UPDATES:
            case ApiInfo.API_PP_RTK_REQUEST_PRECISE_LOCATION_UPDATES:
            case ApiInfo.API_PP_EDGNSS_UPDATE_NTRIP_GGA_CONSENT:
            case ApiInfo.API_PP_EDGNSS_ENABLE_PPE_NTRIP_STREAM:
            case ApiInfo.API_PP_EDGNSS_REGISTER_AS_CORRECTION_DATA_SOURCE:
            case ApiInfo.API_PP_EDGNSS_INJECT_CORRECTION_DATA:
                fetchArgumentsActivity.launch(mApiToExecute);
                break;

            default:
                logEvent("Invalid API: " + mApiToExecute.apiName);
        }
    }

    // Execute the prepared API, for which arguments have been parsed
    private void executePreparedApi() {

        executePreparedApi(mApiToExecute.apiName);
    }
    private void executePreparedApi(String apiName) {

        if (apiName == null) {
            logEvent("Please select an API");
            return;
        }

        mApiToExecute.apiName = apiName;

        logEvent("Going to execute " + apiName);

        try {
            if (apiName.equals(ApiInfo.API_IQESDK_INIT)) {
                initQesdk();
                logEvent("initQesdk execution completed.");
            } else if (ApiInfo.isEdgnssApi(apiName)) {
                if (beforeEdgnssApiCheck()) {
                    PP_eDGNSSEnums.LocationStatus status = executePreparedEdgnssApi();
                    logEvent(apiName + " returned " + status);
                } else {
                    logEvent("beforeEdgnssApiCheck failed.");
                }
            } else if (ApiInfo.isRtkApi(apiName)) {
                if (beforeRtkApiCheck()) {
                    PP_RTKEnums.LocationStatus status = executePreparedRtkApi();
                    logEvent(apiName + " returned " + status);
                } else {
                    logEvent("beforeRtkApiCheck failed.");
                }
            }
        } catch (QesdkStatusException e) {
            logEvent("Exception while invoking: " + apiName);
            logEvent(e.getMessage());
        }
    }

    private PP_eDGNSSEnums.LocationStatus executePreparedEdgnssApi() throws QesdkStatusException {

        switch (mApiToExecute.apiName) {
            case ApiInfo.API_PP_EDGNSS_REQUEST_PRECISE_LOCATION_UPDATES:
                return mPpeEdgnssManager.requestPreciseLocationUpdates(
                        mApiToExecute.minIntervalMillis, eDgnssLocationReportCallback);
            case ApiInfo.API_PP_EDGNSS_REGISTER_LOCATION_CAPABILITIES_CB:
                return mPpeEdgnssManager.registerLocationCapabilitiesCallback(
                        locationCapabilitiesCallback);
            case ApiInfo.API_PP_EDGNSS_REMOVE_PRECISE_LOCATION_UPDATES:
                return mPpeEdgnssManager.removePreciseLocationUpdates(
                        eDgnssLocationReportCallback);
            case ApiInfo.API_PP_EDGNSS_DISABLE_PPE_NTRIP_STREAM:
                return mPpeEdgnssManager.disablePPENtripStream();
            case ApiInfo.API_PP_EDGNSS_DEREGISTER_AS_CORRECTION_DATA_SOURCE:
                return mPpeEdgnssManager.deRegisterAsCorrectionDataSource();
            case ApiInfo.API_PP_EDGNSS_UPDATE_NTRIP_GGA_CONSENT:
                return mPpeEdgnssManager.updateNTRIPGGAConsent(
                        mApiToExecute.ntripGGAConsentAccepted);
            case ApiInfo.API_PP_EDGNSS_ENABLE_PPE_NTRIP_STREAM:
                return mPpeEdgnssManager.enablePPENtripStream(
                        mApiToExecute.hostNameOrIp, mApiToExecute.mountPoint,
                        mApiToExecute.username, mApiToExecute.password, mApiToExecute.port,
                        mApiToExecute.requiresNmeaLocation, mApiToExecute.useSSL,
                        mApiToExecute.enableRTKEngine);
            case ApiInfo.API_PP_EDGNSS_REGISTER_AS_CORRECTION_DATA_SOURCE:
                return mPpeEdgnssManager.registerAsCorrectionDataSource(
                        mApiToExecute.correctionDataType, streamingControlCallback);
            case ApiInfo.API_PP_EDGNSS_INJECT_CORRECTION_DATA:
                //Don't execute this API manually, otherwise it would crash
                //Only use this UI to set Java Ntrip parameters
                logEvent("Java Ntrip client paramters set!");
                //return mPpeEdgnssManager.injectCorrectionData(mApiToExecute.correctionData);
                break;
            default:
                logEvent("Unsupported API: " + mApiToExecute.apiName);
        }

        return PP_eDGNSSEnums.LocationStatus.EDGNSS_LOCATION_STATUS_FAILURE;
    }

    private PP_RTKEnums.LocationStatus executePreparedRtkApi() throws QesdkStatusException {

        switch (mApiToExecute.apiName) {
            case ApiInfo.API_PP_RTK_REQUEST_PRECISE_LOCATION_UPDATES:
                return mPpeRtkManager.requestPreciseLocationUpdates(
                        mApiToExecute.minIntervalMillis, rtkLocationReportCallback);
            case ApiInfo.API_PP_RTK_REMOVE_PRECISE_LOCATION_UPDATES:
                return mPpeRtkManager.removePreciseLocationUpdates(rtkLocationReportCallback);
            default:
                logEvent("Unsupported API: " + mApiToExecute.apiName);
        }

        return PP_RTKEnums.LocationStatus.RTK_LOCATION_STATUS_FAILURE;
    }

    // Initialize QESDK Manager
    private void initQesdk() {

        if (mQesdkManager == null) {

            mQesdkManager = IQesdk.createInstance(this);
            logEvent("IQesdk instance created, now invoking init.");
            int sessionId = mQesdkManager.init(new IQesdkEventCallBack() {
                @Override
                public void onEvent(int i, int[] ints) {

                    logEvent("IQesdkEventCallback.onEvent - " + i);
                }
            });
            logEvent("Got session ID: " + sessionId);

        } else {
            logEvent("IQesdk instance already created.");
        }
    }

    // Before API checks
    private boolean beforeEdgnssApiCheck() {

        if (mQesdkManager == null) {
            logEvent("Please initialize IQesdk first.");
            return false;
        }
        if (mPpeEdgnssManager == null) {
            logEvent("Going to create PPE_eDGNSSManager instance.");
            mPpeEdgnssManager = new PP_eDGNSSManager(mQesdkManager);
        } else {
            logEvent("PPE_eDGNSSManager instance already created.");
        }

        return true;
    }

    private boolean beforeRtkApiCheck() {

        if (mQesdkManager == null) {
            logEvent("Please initialize IQesdk first.");
            return false;
        }
        if (mPpeRtkManager == null) {
            logEvent("Going to create PPE_RTKManager instance.");
            mPpeRtkManager = new PP_RTKManager(mQesdkManager);
        } else {
            logEvent("PPE_RTKManager instance already created.");
        }

        return true;
    }

    // CALLBACKS
    public void capabilitiesCb(long capabilitiesMask) {

        // Values must be from LocationCapabilities enum
        logEvent("CB :: ILocationCapabilitiesCallback() capabilitiesMask = " + capabilitiesMask);

        if ((capabilitiesMask & PP_eDGNSSEnums.LocationCapabilities.
                PP_LOCATION_CAPABILITIES_EDGNSS_BIT.getValue()) != 0) {
            logEvent("capabilitiesMask contains PP_LOCATION_CAPABILITIES_EDGNSS_BIT");
        }
        if ((capabilitiesMask & PP_eDGNSSEnums.LocationCapabilities.
                PP_LOCATION_CAPABILITIES_RTK_BIT.getValue()) != 0) {
            logEvent("capabilitiesMask contains PP_LOCATION_CAPABILITIES_RTK_BIT");
        }
    }

    public void correctionStreamingControlCb(PP_eDGNSSEnums.CorrectionStreamingState state) {

        logEvent("CB :: ICorrectionStreamingControlCallback() state = " + state);

        switch (state) {
            case CORRECTION_STREAMING_STATE_START:
                if (true) {
                    mNtripClient.startCorrectionDataStreaming(
                            mApiToExecute.appNtripUseNtrip2Version,
                            mApiToExecute.appNtripHostName,
                            mApiToExecute.appNtripMountPoint,
                            mApiToExecute.appNtripUsernamePwdEncodedInBase64Format,
                            mApiToExecute.appNtripPort,
                            null,
                            appNtripClientCorrectionDataCb);
                } else {
                    logEvent("App NTRIP client disabled, ignoring injection start request.");
                }
                break;

            case CORRECTION_STREAMING_STATE_STOP:
                if (true) {
                    mNtripClient.stopCorrectionDataStreaming();
                } else {
                    logEvent("App NTRIP client disabled, ignoring injection stop request.");
                }
                break;

            default:
                logEvent("Invalid state: " + state);
                break;
        }
    }

    public void eDgnssLocationReportCb(
            int locationFlags, long timestamp, double latitude, double longitude, double altitude,
            float speed, float bearing, float accuracy, float verticalAccuracy, float speedAccuracy,
            float bearingAccuracy, long elapsedRealTimeNanos, long elapsedRealTimeUncNanos,
            PP_eDGNSSEnums.LocationQuality locationQuality) {

        logEvent("CB :: IPP_eDGNSSCBs.ILocationReportCallback()");

        logEvent("flags: " + locationFlags + "timestamp: " + timestamp +
                ", lat: " + latitude + ", lon: " + longitude + ", alt: " + altitude +
                ", speed: " + speed + ", bearing: " + bearing + ", acc: " + accuracy +
                ", vertAcc: " + verticalAccuracy + ", speedAcc: " + speedAccuracy +
                ", bearingAcc: " + bearingAccuracy +
                ", elapsedRealTimeNanos: " + elapsedRealTimeNanos +
                ", elapsedRealTimeUncNanos" + elapsedRealTimeUncNanos +
                ", locationQuality: " + locationQuality);
        mNtripClient.sendGGANmea(latitude, longitude, altitude);
    }

    public void rtkLocationReportCb(
            int locationFlags, long timestamp, double latitude, double longitude, double altitude,
            float speed, float bearing, float accuracy, float verticalAccuracy, float speedAccuracy,
            float bearingAccuracy, long elapsedRealTimeNanos, long elapsedRealTimeUncNanos,
            PP_RTKEnums.LocationQuality locationQuality) {

        logEvent("CB :: IPP_RTKCBs.ILocationReportCallback()");

        logEvent("flags: " + locationFlags + "timestamp: " + timestamp +
                ", lat: " + latitude + ", lon: " + longitude + ", alt: " + altitude +
                ", speed: " + speed + ", bearing: " + bearing + ", acc: " + accuracy +
                ", vertAcc: " + verticalAccuracy + ", speedAcc: " + speedAccuracy +
                ", bearingAcc: " + bearingAccuracy +
                ", elapsedRealTimeNanos: " + elapsedRealTimeNanos +
                ", elapsedRealTimeUncNanos" + elapsedRealTimeUncNanos +
                ", locationQuality: " + locationQuality);
        mNtripClient.sendGGANmea(latitude, longitude, altitude);
    }
}