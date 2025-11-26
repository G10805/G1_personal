/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.dctestapp;

import android.os.Bundle;
import android.os.RemoteException;
import android.util.Log;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.CheckBox;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.Button;

import androidx.annotation.NonNull;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.Fragment;

import com.google.android.material.textfield.TextInputEditText;

import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;

import vendor.qti.imsdatachannel.aidl.ImsDataChannelAttributes;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelResponse;

public class DcSetupRequestFragment extends DialogFragment {

    final String LOG_TAG = "DCTestApp:DcSetupRequestFragment";

    private SubscriptionFragment mSubscriptionFragment = null;

    private ImsDataChannelAttributes[] mAttrs;
    private ImsDataChannelResponse[] r;
    private Button sendResButton;
    private CheckBox[] boxes;
    private int mReqNum;

    // Executor mExecutor = new ScheduledThreadPoolExecutor(1);

    public DcSetupRequestFragment() {
        // Required empty public constructor
    }

    public DcSetupRequestFragment(SubscriptionFragment subscriptionFragment, ImsDataChannelAttributes[] attrs, int reqNum) {
      mSubscriptionFragment = subscriptionFragment;
      mAttrs = attrs;
      mReqNum = reqNum;
    }

    public void onCallEnded() {
        if (sendResButton != null) {
            sendResButton.post(() -> {
                sendResButton.setEnabled(false);
            });
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(
            @NonNull LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        View root = inflater.inflate(R.layout.fragment_dc_setup_request, container, false);

        ViewGroup attrsContainer = root.findViewById(R.id.linearLayout_dcSetupRequest);

        r = new ImsDataChannelResponse[mAttrs.length];
        boxes = new CheckBox[mAttrs.length];
        for (int i = 0; i < mAttrs.length; i++) {
            boxes[i] = new CheckBox(getActivity(), null, 0, R.style.AttrCheckBox);
            boxes[i].setText(mAttrs[i].getDcId());
            boxes[i].setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT,
                                                     LayoutParams.WRAP_CONTENT));
            attrsContainer.addView(boxes[i]);
            // TextView msgView = new TextView(getActivity());

            r[i] = new ImsDataChannelResponse();
            r[i].setDcId(mAttrs[i].getDcId());
            r[i].setAcceptStatus(false);
        }

        sendResButton = root.findViewById(R.id.button_sendResponse);
        sendResButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                for (int i = 0; i < r.length; i++) {
                    r[i].setAcceptStatus(boxes[i].isChecked());
                    Log.d(LOG_TAG, "sendResButton onClick: r[" + i + "] set to " + boxes[i].isChecked());
                }
                mSubscriptionFragment.respondToDataChannelSetUpRequest(r, mReqNum);
                dismiss();
            }
        });

        return root;
    }

    @Override
    public void onResume() {
        super.onResume();
        ViewGroup.LayoutParams params = getDialog().getWindow().getAttributes();
        params.width = LayoutParams.MATCH_PARENT;
        params.height = LayoutParams.MATCH_PARENT;
        getDialog().getWindow().setAttributes((android.view.WindowManager.LayoutParams) params);
    }

}
