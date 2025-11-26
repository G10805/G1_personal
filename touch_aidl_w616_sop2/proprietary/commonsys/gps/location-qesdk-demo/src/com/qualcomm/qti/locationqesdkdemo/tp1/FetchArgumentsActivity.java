/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
package com.qualcomm.qti.locationqesdkdemo.tp1;

import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.qualcomm.qti.locationqesdkdemo.tp1.R;
import com.qualcomm.qti.qesdk.Location.PP_eDGNSSEnums;

import java.io.Serializable;

public class FetchArgumentsActivity extends AppCompatActivity {

    private final String TAG = "LocationQESDK.FetchArgumentsActivity";

    private ApiInfo mApiInfo;

    // Views for different arguments
    TextView minIntervalMillisLabelTextView;
    EditText minIntervalMillisEditText;

    CheckBox ntripGGAConsentCheckBox;

    TextView hostNameOrIpLabelTextView;
    EditText hostNameOrIpEditText;

    TextView mountPointLabelTextView;
    EditText mountPointEditText;

    TextView usernameLabelTextView;
    EditText usernameEditText;

    TextView passwordLabelTextView;
    EditText passwordEditText;

    TextView portLabelTextView;
    EditText portEditText;

    CheckBox requiresNmeaLocationCheckBox;
    CheckBox useSslCheckBox;
    CheckBox enableRtkEngineCheckBox;

    TextView correctionDataTypeLabelTextView;
    Spinner correctionDataTypeSpinner;

    CheckBox appNtripClientEnabled;
    CheckBox appNtripUseNtrip2Version;
    TextView appNtripHostNameOrIpLabel;
    EditText appNtripHostNameOrIp;
    TextView appNtripMountPointLabel;
    EditText appNtripMountPoint;
    TextView appNtripUsernamePwdBase64EncodedLabel;
    EditText appNtripUsernamePwdBase64Encoded;
    TextView appNtripPortLabel;
    EditText appNtripPort;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_fetch_arguments);

        mApiInfo = (ApiInfo) getIntent().getSerializableExtra("api_info");
        if (mApiInfo == null) {
            Log.i(TAG, "null apiInfo received.");
            setResult(RESULT_CANCELED);
            finish();
        }

        setupViewInstances();
        setupInputViewsAsPerApi();

        Button executeApiButton = findViewById(R.id.submitButton);
        executeApiButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                sendBackArguments();
            }
        });
    }

    private void setupViewInstances() {

        minIntervalMillisLabelTextView = findViewById(R.id.minIntervalMillisLabel);
        minIntervalMillisEditText = findViewById(R.id.minIntervalMillis);

        ntripGGAConsentCheckBox = findViewById(R.id.ntripGGAConsent);

        hostNameOrIpLabelTextView = findViewById(R.id.hostNameOrIpLabel);
        hostNameOrIpEditText = findViewById(R.id.hostNameOrIp);

        mountPointLabelTextView = findViewById(R.id.mountPointLabel);
        mountPointEditText = findViewById(R.id.mountPoint);

        usernameLabelTextView = findViewById(R.id.usernameLabel);
        usernameEditText = findViewById(R.id.username);

        passwordLabelTextView = findViewById(R.id.passwordLabel);
        passwordEditText = findViewById(R.id.password);

        portLabelTextView = findViewById(R.id.portLabel);
        portEditText = findViewById(R.id.port);

        requiresNmeaLocationCheckBox = findViewById(R.id.requiresNmeaLocation);
        useSslCheckBox = findViewById(R.id.useSsl);
        enableRtkEngineCheckBox = findViewById(R.id.enableRtkEngine);

        correctionDataTypeLabelTextView = findViewById(R.id.correctionDataTypeLabel);
        correctionDataTypeSpinner = findViewById(R.id.correctionDataType);

        appNtripClientEnabled = findViewById(R.id.appNtripClientEnabled);
        appNtripUseNtrip2Version = findViewById(R.id.appNtripUseNtrip2Version);
        appNtripHostNameOrIpLabel = findViewById(R.id.appNtripHostNameOrIpLabel);
        appNtripHostNameOrIp = findViewById(R.id.appNtripHostNameOrIp);
        appNtripMountPointLabel = findViewById(R.id.appNtripMountPointLabel);
        appNtripMountPoint = findViewById(R.id.appNtripMountPoint);
        appNtripUsernamePwdBase64EncodedLabel = findViewById(R.id.appNtripUsernamePwdBase64EncodedLabel);
        appNtripUsernamePwdBase64Encoded = findViewById(R.id.appNtripUsernamePwdBase64Encoded);
        appNtripPortLabel = findViewById(R.id.appNtripPortLabel);
        appNtripPort = findViewById(R.id.appNtripPort);
    }

    private void hideAllViews() {

        minIntervalMillisLabelTextView.setVisibility(View.GONE);
        minIntervalMillisEditText.setVisibility(View.GONE);

        ntripGGAConsentCheckBox.setVisibility(View.GONE);

        hostNameOrIpLabelTextView.setVisibility(View.GONE);
        hostNameOrIpEditText.setVisibility(View.GONE);
        mountPointLabelTextView.setVisibility(View.GONE);
        mountPointEditText.setVisibility(View.GONE);
        usernameLabelTextView.setVisibility(View.GONE);
        usernameEditText.setVisibility(View.GONE);
        passwordLabelTextView.setVisibility(View.GONE);
        passwordEditText.setVisibility(View.GONE);
        portLabelTextView.setVisibility(View.GONE);
        portEditText.setVisibility(View.GONE);
        requiresNmeaLocationCheckBox.setVisibility(View.GONE);
        useSslCheckBox.setVisibility(View.GONE);
        enableRtkEngineCheckBox.setVisibility(View.GONE);

        correctionDataTypeLabelTextView.setVisibility(View.GONE);
        correctionDataTypeSpinner.setVisibility(View.GONE);

        appNtripClientEnabled.setVisibility(View.GONE);
        appNtripUseNtrip2Version.setVisibility(View.GONE);
        appNtripHostNameOrIpLabel.setVisibility(View.GONE);
        appNtripHostNameOrIp.setVisibility(View.GONE);
        appNtripMountPointLabel.setVisibility(View.GONE);
        appNtripMountPoint.setVisibility(View.GONE);
        appNtripUsernamePwdBase64EncodedLabel.setVisibility(View.GONE);
        appNtripUsernamePwdBase64Encoded.setVisibility(View.GONE);
        appNtripPortLabel.setVisibility(View.GONE);
        appNtripPort.setVisibility(View.GONE);
    }

    private void setupInputViewsAsPerApi() {

        // Setup title in all cases
        TextView titleView = findViewById(R.id.titleTextView);
        titleView.setText(mApiInfo.apiName + " Arguments");

        setupCorrectionDataTypeSpinner();

        // Hide all view and then enable as per API
        hideAllViews();

        switch (mApiInfo.apiName) {

            case ApiInfo.API_PP_EDGNSS_REQUEST_PRECISE_LOCATION_UPDATES:
            case ApiInfo.API_PP_RTK_REQUEST_PRECISE_LOCATION_UPDATES:
                minIntervalMillisLabelTextView.setVisibility(View.VISIBLE);
                minIntervalMillisEditText.setVisibility(View.VISIBLE);
                break;

            case ApiInfo.API_PP_EDGNSS_UPDATE_NTRIP_GGA_CONSENT:
                ntripGGAConsentCheckBox.setVisibility(View.VISIBLE);
                break;

            case ApiInfo.API_PP_EDGNSS_ENABLE_PPE_NTRIP_STREAM:
                hostNameOrIpLabelTextView.setVisibility(View.VISIBLE);
                hostNameOrIpEditText.setVisibility(View.VISIBLE);
                mountPointLabelTextView.setVisibility(View.VISIBLE);
                mountPointEditText.setVisibility(View.VISIBLE);
                usernameLabelTextView.setVisibility(View.VISIBLE);
                usernameEditText.setVisibility(View.VISIBLE);
                passwordLabelTextView.setVisibility(View.VISIBLE);
                passwordEditText.setVisibility(View.VISIBLE);
                portLabelTextView.setVisibility(View.VISIBLE);
                portEditText.setVisibility(View.VISIBLE);
                requiresNmeaLocationCheckBox.setVisibility(View.VISIBLE);
                useSslCheckBox.setVisibility(View.VISIBLE);
                enableRtkEngineCheckBox.setVisibility(View.VISIBLE);
                break;

            case ApiInfo.API_PP_EDGNSS_REGISTER_AS_CORRECTION_DATA_SOURCE:
                correctionDataTypeLabelTextView.setVisibility(View.VISIBLE);
                correctionDataTypeSpinner.setVisibility(View.VISIBLE);
                break;

            case ApiInfo.API_PP_EDGNSS_INJECT_CORRECTION_DATA:
                appNtripClientEnabled.setVisibility(View.VISIBLE);
                appNtripUseNtrip2Version.setVisibility(View.VISIBLE);
                appNtripHostNameOrIpLabel.setVisibility(View.VISIBLE);
                appNtripHostNameOrIp.setVisibility(View.VISIBLE);
                appNtripMountPointLabel.setVisibility(View.VISIBLE);
                appNtripMountPoint.setVisibility(View.VISIBLE);
                appNtripUsernamePwdBase64EncodedLabel.setVisibility(View.VISIBLE);
                appNtripUsernamePwdBase64Encoded.setVisibility(View.VISIBLE);
                appNtripPortLabel.setVisibility(View.VISIBLE);
                appNtripPort.setVisibility(View.VISIBLE);
                break;

            default:
                Log.e(TAG, "Invalid API to parse arguments: " + mApiInfo.apiName);
        }
    }

    private boolean parseArgumentsAndUpdateApiInfo() {

        try {

            switch (mApiInfo.apiName) {

                case ApiInfo.API_PP_EDGNSS_REQUEST_PRECISE_LOCATION_UPDATES:
                case ApiInfo.API_PP_RTK_REQUEST_PRECISE_LOCATION_UPDATES:
                    mApiInfo.minIntervalMillis = Integer.parseInt("" + minIntervalMillisEditText.getText());
                    break;

                case ApiInfo.API_PP_EDGNSS_UPDATE_NTRIP_GGA_CONSENT:
                    mApiInfo.ntripGGAConsentAccepted = ntripGGAConsentCheckBox.isChecked();
                    break;

                case ApiInfo.API_PP_EDGNSS_ENABLE_PPE_NTRIP_STREAM:
                    mApiInfo.hostNameOrIp = "" + hostNameOrIpEditText.getText();
                    mApiInfo.mountPoint = "" + mountPointEditText.getText();
                    mApiInfo.username = "" + usernameEditText.getText();
                    mApiInfo.password = "" + passwordEditText.getText();
                    mApiInfo.port = Integer.parseInt("" + portEditText.getText());
                    mApiInfo.requiresNmeaLocation = requiresNmeaLocationCheckBox.isChecked();
                    mApiInfo.useSSL = useSslCheckBox.isChecked();
                    mApiInfo.enableRTKEngine = enableRtkEngineCheckBox.isChecked();
                    break;

                case ApiInfo.API_PP_EDGNSS_INJECT_CORRECTION_DATA:
                    mApiInfo.appNtripClientEnabled = appNtripClientEnabled.isChecked();
                    mApiInfo.appNtripUseNtrip2Version = appNtripUseNtrip2Version.isChecked();
                    mApiInfo.appNtripHostName = "" + appNtripHostNameOrIp.getText();
                    mApiInfo.appNtripPort = Integer.parseInt("" + appNtripPort.getText());
                    mApiInfo.appNtripMountPoint = "" + appNtripMountPoint.getText();
                    mApiInfo.appNtripUsernamePwdEncodedInBase64Format =
                            "" + appNtripUsernamePwdBase64Encoded.getText();
                    break;

                case ApiInfo.API_PP_EDGNSS_REGISTER_AS_CORRECTION_DATA_SOURCE:
                    // Populated in setupCorrectionDataTypeSpinner
                    break;

                default:
                    Log.e(TAG, "Invalid API to parse arguments: " + mApiInfo.apiName);
            }

        } catch (Exception e) {

            Log.e(TAG, "Exception parsing arguments.\n" + e.getMessage());
            return false;
        }

        return true;
    }

    private void sendBackArguments() {

        Log.i(TAG, "sending back arguments..");

        if (!parseArgumentsAndUpdateApiInfo()) {
            Toast.makeText(this, "Invalid argument values, please check.",
                    Toast.LENGTH_LONG).show();
            return;
        }

        Intent intent = new Intent();
        intent.putExtra("api_info", (Serializable) mApiInfo);

        setResult(RESULT_OK, intent);
        finish();
    }

    private void setupCorrectionDataTypeSpinner() {

        Spinner spinner = findViewById(R.id.correctionDataType);
        spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int index, long l) {

                if (index == 1) mApiInfo.correctionDataType =
                        PP_eDGNSSEnums.CorrectionDataType.CORRECTION_DATA_TYPE_EDGNSS;
                if (index == 2) mApiInfo.correctionDataType =
                        PP_eDGNSSEnums.CorrectionDataType.CORRECTION_DATA_TYPE_RTK;
            }
            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {}
        });
        String[] correctionDataTypeList = {
                "Please select a value", "CORRECTION_DATA_TYPE_EDGNSS", "CORRECTION_DATA_TYPE_RTK"};
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                this, R.layout.spinner_element, correctionDataTypeList) {
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
}