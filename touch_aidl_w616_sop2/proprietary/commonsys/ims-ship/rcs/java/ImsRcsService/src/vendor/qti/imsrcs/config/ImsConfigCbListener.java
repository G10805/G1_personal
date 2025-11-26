/**********************************************************************
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/


package vendor.qti.imsrcs.config;

import android.telephony.ims.feature.RcsFeature;
import android.util.Log;

import java.util.ArrayList;
import java.util.concurrent.Executor;

import vendor.qti.ims.configservice.V1_0.AutoConfig;
import vendor.qti.ims.configservice.V1_0.AutoConfigResponse;
import vendor.qti.ims.configservice.V1_1.SettingsData;
import vendor.qti.imsrcs.config.ImsConfigServiceWrapper;

/**
 * thread context switch from HAL to ConfigThread, data handling,
 * exposed for AOSP/fw callbacks
 */


public abstract class ImsConfigCbListener {
    private final String LOG_TAG = "ImsConfigCbListener";
    private Executor mExecutor;

    public ImsConfigCbListener(Executor ex){
        this.mExecutor = ex;
    }

    public ImsConfigCbListener(){

    }

   /*
    public void handleRcsStatusChange(boolean isRcsEnabled)
    {

    }

    public void handleUceStatusChange(Executor uceExecutor, RcsFeature.RcsImsCapabilities capabilities, ImsConfigServiceWrapper.ImsConfigUceListener uceCbListener) {
          //updateUceStatus(capabilities);
        uceExecutor.execute(
                () -> {

                }
        );
        //from auto notification
    } */

    public final void notifyCommandStatus(int status, int userData) {
        onCommandStatusCb(status,userData);
        Log.d(LOG_TAG,String.valueOf(status) + "userData: " + String.valueOf(userData));
        return;
        /*mExecutor.execute(
                () ->{
                    Log.d(LOG_TAG,String.valueOf(status) + "userData: " + String.valueOf(userData));

                }
        ); */
    }

    public final void handleGetSettingsResp(int status, SettingsData settingsData, int userData) {
    if(settingsData == null){
       Log.d(LOG_TAG,"handleGetSettingsResp settingsData null");
       return;
       }
        mExecutor.execute(
                () -> {
                     Log.d(LOG_TAG,"handleGetSettingsResp 1_1 cb: " + String.valueOf(settingsData) + "status :" + String.valueOf(status) + "userData: " + String.valueOf(userData));
                }
        );
    }

    public final void handleGetSettingsResp(int status, vendor.qti.ims.configservice.V1_0.SettingsData settingsData, int userData) {
    if(settingsData == null){
       Log.d(LOG_TAG,"handleGetSettingsResp settingsData null");
       return;
       }
        mExecutor.execute(
                () -> {
                     Log.d(LOG_TAG,"handleGetSettingsResp: " + String.valueOf(settingsData) + "status :" + String.valueOf(status) + "userData: " + String.valueOf(userData));
                }
        );
    }

    public final void handleAutoConfigReceived(AutoConfig autoConfig) {
         Log.d(LOG_TAG,"handleAutoConfigReceived inside");
        mExecutor.execute(
                () ->{
                    if(autoConfig == null){
                       Log.d(LOG_TAG,"autoConfig data null");
                       return;
                    }
                    boolean isCompressed = autoConfig.configData.isCompressed;
                    ArrayList<Byte> configData = autoConfig.configData.config;
                    if(configData != null && configData.size()>0)
                    {
                      Log.d(LOG_TAG,"onAutoConfigurationReceivedCb: xml: " + configData.toString());
                    byte[] arr = new byte[configData.size()];
                    for(int i=0;i<configData.size();i++){
                         arr[i] = (byte)configData.get(i);
                    }
                    byte[] tempArr = arr.clone();
                    String configStr = tempArr.toString();
                    //only for VZW, preConfigXml contains unique value param to be 100
                    if(configStr.contains("value=\"100\"")){
                        onPreConfigReceivedCb(arr);
                        Log.d(LOG_TAG,"onPreConfigReceivedCb");
                        return;
                    }
                    onAutoConfigurationReceivedCb(arr,isCompressed);
                      Log.d(LOG_TAG,"onAutoConfigurationReceivedCb");
                    }
                    else
                    {
                       Log.d(LOG_TAG,"handleAutoConfigReceived config data null");
                    }
                }
        );
    }

    public final void handleAutoConfigErrorCb(AutoConfigResponse autoConfigResponse){
        mExecutor.execute(
                () ->{
                    int errorCode = (int)autoConfigResponse.statusCode;
                    String errorStr = autoConfigResponse.reasonPhrase;
                    onAutoConfigurationErrorReceivedCb(errorCode,errorStr);
                }
        );
    }

    public final void handleGetUpdatedSettingsCb(SettingsData settingsData)
    {
       if(settingsData == null){
       Log.d(LOG_TAG,"handleGetUpdatedSettingsCb settingsData null");
       return;
       }
        mExecutor.execute(
                () -> {
                     Log.d(LOG_TAG,"handleGetUpdatedSettingsCb 1_1 cb: " + String.valueOf(settingsData));
                }
        );

    }

    public final void handleGetUpdatedSettingsCb(vendor.qti.ims.configservice.V1_0.SettingsData settingsData)
    {
       if(settingsData == null){
       Log.d(LOG_TAG,"handleGetUpdatedSettingsCb settingsData null");
       return;
       }
        mExecutor.execute(
                () -> {
                     Log.d(LOG_TAG,"handleGetUpdatedSettingsCb: " + String.valueOf(settingsData));
                }
        );

    }

    public final void handleConfigWrapperCleanup()
    {
       Log.d(LOG_TAG,"handleConfigWrapperCleanup");
       onWrapperReset();

    }


    public void onCommandStatusCb(int status, int userData) {

    }

    public void onAutoConfigurationReceivedCb(byte[] autoConfigXml, boolean isCompressed){
    }

    public void onAutoConfigurationErrorReceivedCb(int errorCode, String errorStr){
    }

    public void onPreConfigReceivedCb(byte[] preconfigxml){
    }

    public void onWrapperReset(){
    }


}
