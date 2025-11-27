package com.visteon.rvc.test;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;
import android.view.View;
import android.visteon.rvc.IRvcService;
import android.widget.Toast;

public class MainActivity extends Activity implements View.OnClickListener {

    /**
     * <p>Broadcast Action: Rear camera has started or stopped.
     * The intent will have the following extra value:</p>
     * <ul>
     *   <li><em>state</em> - A boolean value indicating whether Rear view camera is on or off.</li>
     * </ul>
     */
    public static final String ACTION_REAR_CAMERA_STATE_CHANGED = "android.visteon.intent.action.REAR_CAMERA_STATE_CHANGED";

    private static final String RVC_SERVICE = "VSTRvcService";
    private static final String TAG = "TEST_RVC";

    private View mRvcView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mRvcView = findViewById(R.id.camara_output_view);

        findViewById(R.id.button_start_preview).setOnClickListener(this);
        findViewById(R.id.button_stop_preview).setOnClickListener(this);


        IntentFilter filter = new IntentFilter(ACTION_REAR_CAMERA_STATE_CHANGED);
        registerReceiver(mReceiver, filter);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mReceiver);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            startCameraPreview();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        stopCameraPreview();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.button_start_preview:
                startCameraPreview();
            break;
            case R.id.button_stop_preview:
                stopCameraPreview();
            break;
        }
    }

    private void startCameraPreview() {
        final IRvcService vstRvcManager = IRvcService.Stub
                .asInterface(ServiceManager.getService(RVC_SERVICE));
        if (vstRvcManager != null) {
            try {
                boolean state = vstRvcManager.hasStarted();
                Log.d(TAG, "startCameraPreview: RVC " + (state ? "STARTED" : "STOPPED"));

                int START_MULTIPLE = 10; // MUST BE Multiple of 5 or 10 pixels for x and y co-ordinates
                int END_MULTIPLE = 64; // MUST BE Multiple of 64 pixels for height and width
                int left = (int)mRvcView.getX() / START_MULTIPLE * START_MULTIPLE;
                int top = (int)mRvcView.getY() / START_MULTIPLE * START_MULTIPLE;
                int right = left + mRvcView.getWidth() / END_MULTIPLE * END_MULTIPLE;
                int bottom = top + mRvcView.getHeight() / END_MULTIPLE * END_MULTIPLE;
                Log.d(TAG, "left=" + left);
                Log.d(TAG, "top=" + top);
                Log.d(TAG, "right=" + right);
                Log.d(TAG, "bottom=" + bottom);
                vstRvcManager.setClipBounds(left, top, right, bottom);
                vstRvcManager.startPreview();
            } catch (RemoteException e) {
                Log.w(TAG, e.getMessage(), e);
            }
        } else {
            Log.d(TAG, "Unable to startPreview <IRvcService is null>");
            Toast.makeText(getApplicationContext(), "Unable to startPreview <IRvcService is null>",
                    Toast.LENGTH_LONG).show();
        }
    }

    private void stopCameraPreview() {
        final IRvcService vstRvcManager = IRvcService.Stub
                .asInterface(ServiceManager.getService(RVC_SERVICE));
        if (vstRvcManager != null) {
            Log.d(TAG, "stopCameraPreview");
            try {
                vstRvcManager.stopPreview();
            } catch (RemoteException e) {
                Log.w(TAG, e.getMessage(), e);
            }
        } else {
            Log.d(TAG, "Unable to stopPreview <IRvcService is null>");
            Toast.makeText(getApplicationContext(), "Unable to stopPreview <IRvcService is null>",
                    Toast.LENGTH_LONG).show();
        }
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            boolean state = intent.getBooleanExtra("state", false);
            Log.d(TAG, "BroadcastReceiver: RVC " + (state ? "STARTED" : "STOPPED"));
        }
    };
}
