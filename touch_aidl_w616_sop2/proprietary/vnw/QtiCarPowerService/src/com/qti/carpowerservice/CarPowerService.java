/******************************************************************************
 @file CarPowerService.java
 @brief Detaches and Attaches usb devices on LPM

 ---------------------------------------------------------------------------
 Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 All rights reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ---------------------------------------------------------------------------
 ******************************************************************************/

package com.qti.carpowerservice;

import android.app.Service;
import android.car.Car;
import android.car.hardware.power.CarPowerManager;
import android.car.hardware.power.CarPowerManager.CarPowerStateListenerWithCompletion;
import android.car.hardware.power.CarPowerManager.CompletablePowerStateChangeFuture;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashMap;

public class CarPowerService extends Service implements CarPowerStateListenerWithCompletion {

    static final String TAG = "QtiCarPowerService";

    private static final String SYSFS_XHCI_PATH = "/sys/bus/platform/drivers/xhci-hcd/";
    private static final String SYSFS_BIND_NODE = SYSFS_XHCI_PATH + "bind";
    private static final String SYSFS_UNBIND_NODE = SYSFS_XHCI_PATH + "unbind";
    private static final int USB_DETACH_WAIT_DURATION_MS = 2 * 1000; // 2 sec

    private static Car mCar;
    private CarPowerManager mCarPowerManager;
    private HashMap<String, String> mUsbDeviceMap;
    private String[] mXhciNodes = null;

    private final Car.CarServiceLifecycleListener mCarServiceLifecycleListener =
            (car, ready) -> {
                Log.i(TAG, "Car LifecycleCallback ready:" + ready);
                if (!ready) {
                    mCarPowerManager = null;
                    return;
                }
                mCar = car;
                mCarPowerManager = (CarPowerManager) car.getCarManager(
                        Car.POWER_SERVICE);
                Log.i(TAG, "Registering for Power events");
                mCarPowerManager.setListenerWithCompletion(this.getMainExecutor(),
                        CarPowerService.this);
            };

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "CarPowerService started");
        mUsbDeviceMap = new HashMap<String, String>();
        Car car = Car.createCar(this, null, Car.CAR_WAIT_TIMEOUT_DO_NOT_WAIT, mCarServiceLifecycleListener);

        // generic
        addIfUsbNodeExists("/sys/devices/platform/soc/a400000.ssusb/mode");
        addIfUsbNodeExists("/sys/devices/platform/soc/a600000.ssusb/mode");
        addIfUsbNodeExists("/sys/devices/platform/soc/a800000.ssusb/mode");
        // gen4
        addIfUsbNodeExists("/sys/devices/platform/soc/a400000.hsusb/mode");
        // Talos specific
        addIfUsbNodeExists("/sys/devices/platform/soc/a800000.hsusb/mode");
    }

    private void addIfUsbNodeExists(String path) {
        File file = new File(path);
        if (file.exists()) {
            Log.i(TAG, "Adding usb node to map: " + path);
            mUsbDeviceMap.put(path, "");
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mCarPowerManager != null) {
            mCarPowerManager.clearListener();
        }
    }

    @Override
    public final int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
    }

    @Override
    public final IBinder onBind(Intent intent) {
        return null;
    }

    private void writeToDevice(String device, String value) {
        Log.i(TAG, "write " + value + " -> " + device);
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(device))) {
            writer.write(value);
            writer.flush();
        } catch (IOException e) {
            Log.e(TAG, "Failed to write to device: " + device);
            Log.e(TAG, "Error: " + e);
        }
    }

    private String readFromDevice(String device) {
        StringBuilder mode = new StringBuilder();
        try (BufferedReader reader = new BufferedReader(new FileReader(device))) {
            String line;
            while ((line = reader.readLine()) != null) {
                mode.append(line);
            }
        } catch (IOException e) {
            Log.e(TAG, "Failed to read from device: " + e);
        }
        Log.i(TAG, "read " + device + ": " + mode);
        return mode.toString();
    }

    private void detachUsbDevices() {
        Log.i(TAG, "detachUsbDevices");
        for (String key : mUsbDeviceMap.keySet()) {
            String curMode = readFromDevice(key);
            mUsbDeviceMap.put(key, curMode);
            writeToDevice(key, "none");
        }
    }

    private void attachUsbDevices() {
        Log.i(TAG, "attachUsbDevices");
        for (String key : mUsbDeviceMap.keySet()) {
            // skip if mode is empty
            String curMode = mUsbDeviceMap.get(key);
            if (curMode.equals(""))
                continue;
            writeToDevice(key, curMode);
            mUsbDeviceMap.put(key, "");
        }
    }

    // Opens SYSFS_XHCI_PATH and returns list of files starting with "xhci-hcd"
    private String[] getXhciNodes() {
        String[] xhciNodes = null;
        File directory = new File(SYSFS_XHCI_PATH);
        if (!directory.exists()) {
            Log.w(TAG, "Directory doesnt exist: " + SYSFS_XHCI_PATH);
        } else {
            xhciNodes = directory.list((dir, name) -> name.startsWith("xhci-hcd"));
            if (xhciNodes == null || xhciNodes.length == 0)
                Log.e(TAG, "No 'xhci-hcd' nodes found");
        }
        return xhciNodes;
    }

    private void updateXhciNodes(boolean bind) {
        final String bindNode = bind ? SYSFS_BIND_NODE : SYSFS_UNBIND_NODE;
        // Collect nodes before unbind
        // On bind, we write them back
        if (!bind /* unbind */)
            mXhciNodes = getXhciNodes();
        if (mXhciNodes == null)
            return;
        for (String value : mXhciNodes) {
            writeToDevice(bindNode, value);
        }
    }

    @Override
    public void onStateChanged(int state, CompletablePowerStateChangeFuture future) {
        Log.v(TAG, "onStateChanged: " + state);
        switch (state) {
            // Lets detach as late as possible
            case CarPowerManager.STATE_POST_SUSPEND_ENTER:
            case CarPowerManager.STATE_POST_HIBERNATION_ENTER:
                Log.i(TAG, "onStateChanged: Entering post suspend state");
                updateXhciNodes(false /*unbind*/);
                detachUsbDevices();
                try {
                    Log.i(TAG, "Waiting " + USB_DETACH_WAIT_DURATION_MS + "ms for usb detach to complete");
                    Thread.sleep(USB_DETACH_WAIT_DURATION_MS);
                } catch (InterruptedException e) {
                    Log.e(TAG, "Sleep Interrupted: " + e);
                }
                break;
            case CarPowerManager.STATE_SUSPEND_EXIT:
            case CarPowerManager.STATE_HIBERNATION_EXIT:
                Log.i(TAG, "onStateChanged: Exit Suspend state");
                attachUsbDevices();
                updateXhciNodes(true /*bind*/);
                break;
            case CarPowerManager.STATE_ON:
                // TODO: if in rare case a crash occurs after SUSPEND_ENTER, there is no way to recover and reattach the usb devices
                // since data will be lost, there is no way to recover. Implment a way to store the prev state and recover in case of crash
            default:
                break;
        }
        if (future != null)
            future.complete();
    }
}
