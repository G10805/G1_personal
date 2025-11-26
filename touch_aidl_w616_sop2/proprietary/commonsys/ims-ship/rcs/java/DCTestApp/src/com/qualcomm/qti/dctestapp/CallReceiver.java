/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.dctestapp;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;
import android.util.Log;
import android.content.IntentFilter;

import java.util.HashMap;
import java.util.Map;

public class CallReceiver {

    final String LOG_TAG = "DCTestApp:CallReceiver";

    final static String MT_CALL_ACTION = "org.codeaurora.intent.action.PRE_ALERTING_CALL_INFO";
    final static String MO_CALL_ACTION = "org.codeaurora.intent.action.DATA_CHANNEL_INFO";

    private Map<Integer, SubscriptionFragment> listenerMap = new HashMap<Integer, SubscriptionFragment>();
    private Context mContext;

    CallReceiver(Context context) {
        mContext = context;
    }

    public final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.i(LOG_TAG, "CallReceiver:onReceive action:" + action);
            int callId = intent.getIntExtra("modemCallId", 0);
            int phoneId = -1;
            if (action.equals(MT_CALL_ACTION))
                phoneId = intent.getIntExtra("pre_alerting_call_phoneId", -1);
            else if (action.equals(MO_CALL_ACTION))
                phoneId = intent.getIntExtra("phoneId", -1);
            SubscriptionFragment listener = listenerMap.get(phoneId);
            if (listener != null) {
                if (callId != 0) {
                    Log.i(LOG_TAG, "CallReceiver callId=" + callId + ", phoneId=" + phoneId);
                    listener.onReceivedCallId(Integer.toString(callId));
                } // else assume call has ended #TODO
            } else {
                Log.e(LOG_TAG, "onReceive invalid phoneId: " + phoneId);
            }
        }
    };

    public void registerForCallIntent() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(MT_CALL_ACTION);
        filter.addAction(MO_CALL_ACTION);
        mContext.registerReceiver(mReceiver, filter);
    }

    public void registerListener(SubscriptionFragment listener) {
        Log.i(LOG_TAG, "CallReceiver: registerListener for slotId=" + listener.getSlotId());
        listenerMap.put(listener.getSlotId(), listener);
    }
}
