/**********************************************************************
 * Copyright (c) 2021-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/


package vendor.qti.imsrcs.config;

import android.app.DownloadManager;
import android.os.RemoteException;
import android.telephony.ims.RcsClientConfiguration;
import android.telephony.ims.feature.RcsFeature;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.Executor;


import vendor.qti.ims.configservice.V1_0.AutoConfig;
import vendor.qti.ims.configservice.V1_0.AutoConfigResponse;
import vendor.qti.ims.configservice.V1_0.ConfigData;
import vendor.qti.ims.configservice.V1_1.IConfigService;
import vendor.qti.ims.configservice.V1_1.IConfigServiceListener;
import vendor.qti.ims.configservice.V1_0.ImsServiceEnableConfigKeys;
import vendor.qti.ims.configservice.V1_0.KeyValuePairTypeBool;
import vendor.qti.ims.configservice.V1_0.KeyValuePairTypeInt;
import vendor.qti.ims.configservice.V1_0.KeyValuePairTypeString;
import vendor.qti.ims.configservice.V1_1.RequestStatus;
import vendor.qti.ims.configservice.V1_1.SettingsData;
import vendor.qti.ims.configservice.V1_0.SettingsId;
import vendor.qti.ims.configservice.V1_1.SettingsValues;
import vendor.qti.ims.configservice.V1_1.StandaloneMessagingConfigKeys;
import vendor.qti.ims.configservice.V1_0.UserAgentStringKeys;
import vendor.qti.ims.configservice.V1_0.UceCapabilityInfo;
import vendor.qti.imsrcs.config.ImsConfigCbListener;
import vendor.qti.imsrcs.config.ImsConfigServiceImpl;
import vendor.qti.imsrcs.config.ImsConfigServiceImpl.QueryAutoConfigListener;

import static android.telephony.ims.feature.RcsFeature.RcsImsCapabilities.CAPABILITY_TYPE_NONE;
import static android.telephony.ims.feature.RcsFeature.RcsImsCapabilities.CAPABILITY_TYPE_OPTIONS_UCE;
import static android.telephony.ims.feature.RcsFeature.RcsImsCapabilities.CAPABILITY_TYPE_PRESENCE_UCE;

public class ImsConfigServiceWrapper {
    IConfigService mHidlConfigService = null;
    vendor.qti.ims.configservice.V1_0.IConfigService mHidlConfigService_1_0 = null;
    ConfigListener mHidlConfigListener = new ConfigListener();
    ConfigListener_1_0 mHidlConfigListener_1_0 = null;
   // ImsConfigServiceImpl.UceCapUpdateCallback mlocalListener;
    private final String LOG_TAG = "ImsConfigServiceWrapper";
    private byte[] cachedConfig;
    private int mUserData = 1234;
    private boolean mIsRcsEnabled = true;
    private ImsConfigCbListener configCbListener;
    //List<Executor> mExecList = new ArrayList<>();
    private Executor uceEx;
    private Executor rcsEx;
    private ImsConfigServiceImpl.UceCapUpdateCallback uceCbListener;
    private ImsConfigServiceImpl.SipTransportCapUpdateCallback rcsCbListener;
    private boolean isRegForInd = false;
    private boolean isModemQmiUp = false;
    private ImsConfigServiceImpl.QueryAutoConfigListener acsQueryListener;
    private ImsConfigServiceImpl.InitQmiRequestsListener initQmiServicesQueryListener;
    private RcsFeature.RcsImsCapabilities uceCaps;
    private RcsClientConfiguration cacheRcsClientConfig;
    private boolean isCachedConfigCompressed=false;
    private String mUserAgentValue = null;

    public void setConfigCbListener(ImsConfigCbListener cb) {
        this.configCbListener = cb;
    }

    public void setUceExecutor(Executor ex){
        this.uceEx = ex;
    }

    public void setRcsExecutor(Executor ex){
        this.rcsEx = ex;
    }

    private Executor getUceExecutor(){
        return uceEx;
    }

    private Executor getRcsExecutor(){
        return rcsEx;
    }

    public void clearConfigCache() {
        cachedConfig = null;
        cacheRcsClientConfig = null;
        uceCaps = null;
    }

    public void clear(){
        clearConfigCache();
        mUserAgentValue = null;
        if(configCbListener != null){
          configCbListener.handleConfigWrapperCleanup();
        }
    }

    public void setHidlConfigService_1_0(vendor.qti.ims.configservice.V1_0.IConfigService s) {
        mHidlConfigService_1_0 = s;
    }
    public  IConfigServiceListener gethidlConfigListener() {
        return mHidlConfigListener;
    }

    public vendor.qti.ims.configservice.V1_0.IConfigServiceListener gethidlConfigListener_1_0() {
       return mHidlConfigListener_1_0;
    }

    public void setHidlConfigService(IConfigService s) {
        mHidlConfigService = s;
    }

    public void createHidlConfigListener_1_0(){
      mHidlConfigListener_1_0 = new ConfigListener_1_0();
    }


    /**
     * this method will call setSettings for SMConfig
     * @param rcc : rcs client configuration
     */
    public int sendRcsClientConfig(RcsClientConfiguration rcc) {
        boolean isRcsEnabled = true;
        cacheRcsClientConfig = rcc;
        int status = -1;
        if(!isModemQmiUp)
          return status;
        ArrayList<KeyValuePairTypeString> strList = new ArrayList<>();

        KeyValuePairTypeString kvStr = new KeyValuePairTypeString();
        kvStr.key = StandaloneMessagingConfigKeys.RCS_VERSION_KEY;
        kvStr.value = rcc.getRcsVersion();
        strList.add(kvStr);

        KeyValuePairTypeString kvStr1 = new KeyValuePairTypeString();
        kvStr1.key = StandaloneMessagingConfigKeys.RCS_PROFILE_KEY;
        kvStr1.value = rcc.getRcsProfile();
        strList.add(kvStr1);

        KeyValuePairTypeString kvStr2 = new KeyValuePairTypeString();
        kvStr2.key = StandaloneMessagingConfigKeys.CLIENT_VENDOR_KEY;
        kvStr2.value = rcc.getClientVendor();
        strList.add(kvStr2);

        KeyValuePairTypeString kvStr3 = new KeyValuePairTypeString();
        kvStr3.key = StandaloneMessagingConfigKeys.CLIENT_VERSION_KEY;
        kvStr3.value = rcc.getClientVersion();
        strList.add(kvStr3);

        mIsRcsEnabled = rcc.isRcsEnabledByUser();

        Log.d(LOG_TAG,"sendRcsClientConfig received Client config: "+ String.valueOf(mIsRcsEnabled));
        getRcsStatus(mIsRcsEnabled);

        /* @test: Adding APP ID parameter here, to be read from client in future */
        /*KeyValuePairTypeString kvStrAppId = new KeyValuePairTypeString();
        kvStrAppId.key = StandaloneMessagingConfigKeys.APP_ID_KEY;
        kvStrAppId.value = "ap2002";
        strList.add(kvStrAppId); */
        if(mHidlConfigService != null){
        SettingsValues values = new SettingsValues();
        values.stringData = strList ;
          status = sendConfigSettings(SettingsId.STANDALONE_MESSAGING_CONFIG,values);
        }
        else if(mHidlConfigService_1_0 != null) {
          vendor.qti.ims.configservice.V1_0.SettingsValues values = new vendor.qti.ims.configservice.V1_0.SettingsValues();
          values.stringData = strList ;
          status = sendConfigSettings(SettingsId.STANDALONE_MESSAGING_CONFIG,values);
        }
        return status;
    }

    /**
     * This method will set UCE status on modem.
     * to be executed under uce executor by UCE module
     * @param isPresenceEnabled
     * @param isOptionsEnabled
     */
    public void setUceStatus(boolean isPresenceEnabled, boolean isOptionsEnabled){

        ArrayList<KeyValuePairTypeBool> boolList = new ArrayList<>();
        KeyValuePairTypeBool kvBool = new KeyValuePairTypeBool();
        kvBool.key = ImsServiceEnableConfigKeys.PRESENCE_ENABLED_KEY;
        kvBool.value = isPresenceEnabled;
        boolList.add(kvBool);
        KeyValuePairTypeBool kvBoolOpt = new KeyValuePairTypeBool();
        kvBoolOpt.key = ImsServiceEnableConfigKeys.OPTIONS_ENABLED_KEY;
        kvBoolOpt.value = isOptionsEnabled;
        boolList.add(kvBoolOpt);
        if(mHidlConfigService != null){
        SettingsValues v = new SettingsValues();
        v.boolData = boolList;
        sendConfigSettings(SettingsId.IMS_SERVICE_ENABLE_CONFIG,v);
    }
        else if(mHidlConfigService_1_0 != null) {
          vendor.qti.ims.configservice.V1_0.SettingsValues v = new vendor.qti.ims.configservice.V1_0.SettingsValues();
          v.boolData = boolList;
          sendConfigSettings(SettingsId.IMS_SERVICE_ENABLE_CONFIG,v);
        }
    }


    public void notifyUceStatus(SettingsValues values)
    {
        boolean isPresenceEnabled=false;
        boolean isOptionsEnabled=false;
        ArrayList<KeyValuePairTypeBool> boolList = values.boolData;
        for(KeyValuePairTypeBool kvBool : boolList){
           if(kvBool.key == ImsServiceEnableConfigKeys.PRESENCE_ENABLED_KEY)
              isPresenceEnabled = kvBool.value;
           if(kvBool.key == ImsServiceEnableConfigKeys.OPTIONS_ENABLED_KEY)
              isOptionsEnabled = kvBool.value;
        }

        UceCapabilityInfo uceCapabilityInfo = new UceCapabilityInfo();
        uceCapabilityInfo.isPresenceEnabled = isPresenceEnabled;
        uceCapabilityInfo.isOptionsEnabled = isOptionsEnabled;

        if(getUceExecutor() != null){
            getUceExecutor().execute(
                    () -> {
                        if(uceCbListener != null)
                        uceCbListener.onUceConfigStatusReceived(getRcsCapabilities(uceCapabilityInfo));
                        else
                           Log.e(LOG_TAG,"onUceConfigStatusReceived received presenceStatus: "+
                     String.valueOf(uceCapabilityInfo.isPresenceEnabled) + "optionsStatus: "
                     + String.valueOf(uceCapabilityInfo.isOptionsEnabled));
                    }
            );
           }
           else
           {
                    Log.e(LOG_TAG,"getUceExecutor null onUceConfigStatusReceived received presenceStatus: "+
                     String.valueOf(uceCapabilityInfo.isPresenceEnabled) + "optionsStatus: "
                     + String.valueOf(uceCapabilityInfo.isOptionsEnabled));
           }


        //return getRcsCapabilities(info);

    }

        public void notifyUceStatus(vendor.qti.ims.configservice.V1_0.SettingsValues values)
    {
        boolean isPresenceEnabled=false;
        boolean isOptionsEnabled=false;
        ArrayList<KeyValuePairTypeBool> boolList = values.boolData;
        for(KeyValuePairTypeBool kvBool : boolList){
           if(kvBool.key == ImsServiceEnableConfigKeys.PRESENCE_ENABLED_KEY)
              isPresenceEnabled = kvBool.value;
           if(kvBool.key == ImsServiceEnableConfigKeys.OPTIONS_ENABLED_KEY)
              isOptionsEnabled = kvBool.value;
        }

        UceCapabilityInfo uceCapabilityInfo = new UceCapabilityInfo();
        uceCapabilityInfo.isPresenceEnabled = isPresenceEnabled;
        uceCapabilityInfo.isOptionsEnabled = isOptionsEnabled;

        if(getUceExecutor() != null){
            getUceExecutor().execute(
                    () -> {
                        if(uceCbListener != null)
                        uceCbListener.onUceConfigStatusReceived(getRcsCapabilities(uceCapabilityInfo));
                        else
                           Log.e(LOG_TAG,"onUceConfigStatusReceived received presenceStatus: "+
                     String.valueOf(uceCapabilityInfo.isPresenceEnabled) + "optionsStatus: "
                     + String.valueOf(uceCapabilityInfo.isOptionsEnabled));
                    }
            );
           }
           else
           {
                    Log.e(LOG_TAG,"getUceExecutor null onUceConfigStatusReceived received presenceStatus: "+
                     String.valueOf(uceCapabilityInfo.isPresenceEnabled) + "optionsStatus: "
                     + String.valueOf(uceCapabilityInfo.isOptionsEnabled));
           }


        //return getRcsCapabilities(info);

    }

    public void notifyRcsStatus(SettingsValues values)
    {
        boolean isRcsEnabled=false;
        //boolean isOptionsEnabled=false;
        ArrayList<KeyValuePairTypeBool> boolList = values.boolData;
        for(KeyValuePairTypeBool kvBool : boolList){
           if(kvBool.key == ImsServiceEnableConfigKeys.RCS_MESSAGING_ENABLED_KEY){
              isRcsEnabled = kvBool.value;
              break;
           }
        }

        if(isRcsEnabled != mIsRcsEnabled) {
            Log.i(LOG_TAG,"UI and modem NV mismatch for RcsMessagingEnabled, so set value as per UI");
            setRcsStatus(mIsRcsEnabled);
            return;
        }

        final boolean rcsstatus = isRcsEnabled;
        if(getRcsExecutor() != null){
            getRcsExecutor().execute(
                    () -> {
                        if(rcsCbListener != null)
                        rcsCbListener.onRcsStatusReceived(rcsstatus);
                        else
                           Log.e(LOG_TAG,"notifyRcsStatus received rcsStatus: "+
                     String.valueOf(rcsstatus) );
                    }
            );
        }
           else
           {
                    Log.e(LOG_TAG,"getRcsExecutor null notifyRcsStatus received rcsStatus: "+
                     String.valueOf(rcsstatus) );
           }

        //return isRcsEnabled;

    }


    public void notifyRcsStatus(vendor.qti.ims.configservice.V1_0.SettingsValues values)
    {
        boolean isRcsEnabled=false;
        //boolean isOptionsEnabled=false;
        ArrayList<KeyValuePairTypeBool> boolList = values.boolData;
        for(KeyValuePairTypeBool kvBool : boolList){
           if(kvBool.key == ImsServiceEnableConfigKeys.RCS_MESSAGING_ENABLED_KEY){
              isRcsEnabled = kvBool.value;
              break;
           }
        }

        final boolean rcsstatus = isRcsEnabled;
        if(getRcsExecutor() != null){
            getRcsExecutor().execute(
                    () -> {
                        if(rcsCbListener != null)
                        rcsCbListener.onRcsStatusReceived(rcsstatus);
                        else
                           Log.e(LOG_TAG,"notifyRcsStatus received rcsStatus: "+
                     String.valueOf(rcsstatus) );
                    }
            );
        }
           else
           {
                    Log.e(LOG_TAG,"getRcsExecutor null notifyRcsStatus received rcsStatus: "+
                     String.valueOf(rcsstatus) );
           }

        //return isRcsEnabled;

    }

    public void setUserAgentValue(SettingsValues values)
    {
        ArrayList<KeyValuePairTypeString> strList = values.stringData;
        for(KeyValuePairTypeString kvStr : strList){
           if(kvStr.key == UserAgentStringKeys.IMS_USER_AGENT_KEY){
              mUserAgentValue = kvStr.value;
              break;
           }
        }
        notifyUserAgentStatusToSipTransport();
    }



    public void setUserAgentValue(vendor.qti.ims.configservice.V1_0.SettingsValues values)
    {
        ArrayList<KeyValuePairTypeString> strList = values.stringData;
        for(KeyValuePairTypeString kvStr : strList){
           if(kvStr.key == UserAgentStringKeys.IMS_USER_AGENT_KEY){
              mUserAgentValue = kvStr.value;
              break;
           }
        }
        notifyUserAgentStatusToSipTransport();
    }

    public void notifyUserAgentStatusToSipTransport(){
        if(mUserAgentValue == null)
           return;
        if(getRcsExecutor() != null){
            getRcsExecutor().execute(
                    () -> {
                        if(rcsCbListener != null)
                          rcsCbListener.onUserAgentStringReceived(mUserAgentValue);
                        else
                           Log.e(LOG_TAG,"notifyUserAgentStatusToSipTransport received mUserAgentValue: "+ mUserAgentValue );
                    }
            );
        }
        else{
            Log.e(LOG_TAG,"getRcsExecutor null notifyUserAgentStatusToSipTransport received mUserAgentValue: "+ mUserAgentValue );
        }
    }

    /**
     * This method will set RcsStatus on modem
     * To be executed on siptransport executor by Sip Transport module
     * @param isRcsEnabled
     */
    public void setRcsStatus(boolean isRcsEnabled){
        ArrayList<KeyValuePairTypeBool> boolList = new ArrayList<>();
        KeyValuePairTypeBool kvBool = new KeyValuePairTypeBool();
        kvBool.key = ImsServiceEnableConfigKeys.RCS_MESSAGING_ENABLED_KEY;
        kvBool.value = isRcsEnabled;
        boolList.add(kvBool);

        if(mHidlConfigService != null){
        SettingsValues v = new SettingsValues();
        v.boolData = boolList;
        sendConfigSettings(SettingsId.IMS_SERVICE_ENABLE_CONFIG,v);
    }
        else if(mHidlConfigService_1_0 != null) {
          vendor.qti.ims.configservice.V1_0.SettingsValues v = new vendor.qti.ims.configservice.V1_0.SettingsValues();
          v.boolData = boolList;
          sendConfigSettings(SettingsId.IMS_SERVICE_ENABLE_CONFIG,v);
        }
    }

    public void triggerAcsRequest(int reasonCode) {
        if(mHidlConfigService != null){
        try {
             Log.d(LOG_TAG,"triggerAcsRequest with ReasonCode: " + String.valueOf(reasonCode));
            mHidlConfigService.triggerAcsRequest(reasonCode,mUserData);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }
        else if(mHidlConfigService_1_0 != null) {
        try {
             Log.d(LOG_TAG,"triggerAcsRequest v1_0 service with ReasonCode: " + String.valueOf(reasonCode));
            mHidlConfigService_1_0.triggerAcsRequest(reasonCode,mUserData);
        } catch (RemoteException e) {
            e.printStackTrace();
        }

        }
    }

        private class ConfigListener_1_0 extends vendor.qti.ims.configservice.V1_0.IConfigServiceListener.Stub {

        @Override
        public void onCommandStatus(int status, int userData) throws RemoteException {
        // v1.0 callback
        if(userData == mUserData && configCbListener!= null)
               configCbListener.notifyCommandStatus(status,userData);
        else
               Log.d(LOG_TAG,"onCommandStatus from diff client, userdata: "+ String.valueOf(userData));
        }

        @Override
        public void onGetSettingsResponse(int status, vendor.qti.ims.configservice.V1_0.SettingsData settingsData, int userData) throws RemoteException {
            //v1.0 callback
            if(settingsData != null){
                 if(settingsData.settingsId == SettingsId.USER_AGENT_STRING){
                  Log.e(LOG_TAG,"onGetSettingsResponse received settingsData USER_AGENT_STRING: " + String.valueOf(settingsData));
                  setUserAgentValue(settingsData.settingsValues);
              }
              //handle other settings
            }
            if(userData == mUserData && configCbListener!= null )
                configCbListener.handleGetSettingsResp(status,settingsData,userData);
            else
                Log.d(LOG_TAG,"onGetSettingsResponse from diff client, userdata: "+ String.valueOf(userData));
            }

        @Override
        public void onAutoConfigurationReceived(AutoConfig autoConfig) throws RemoteException {
            if(configCbListener!= null)
            configCbListener.handleAutoConfigReceived(autoConfig);
        }

        @Override
        public void onReconfigNeeded() throws RemoteException {
         //callback received when qmi service comes up or modem SSR happens
          Log.d(LOG_TAG,"onReconfigNeeded received");
          isModemQmiUp = true;
          /* calling the init values when QMI service is up */
          if(initQmiServicesQueryListener != null)
             initQmiServicesQueryListener.onQmiServiceUp();

        }

        @Override
        public void onTokenFetchRequest(int i, int i1, int i2) throws RemoteException {
          Log.d(LOG_TAG,"onTokenFetchRequest received ");
          /* TODO check here if we want to store them, as service loads later */
        }

        @Override
        public void onUceStatusUpdate(vendor.qti.ims.configservice.V1_0.UceCapabilityInfo uceCapabilityInfo) throws RemoteException {
        Log.d(LOG_TAG,"onUceStatusUpdate received ");
           //configCbListener.handleUceStatusChange(getUceExecutor(),getRcsCapabilities(uceCapabilityInfo),uceCbListener);
           if(getUceExecutor() != null){
            getUceExecutor().execute(
                    () -> {
                        if(uceCbListener != null)
                        uceCbListener.onUceConfigStatusReceived(getRcsCapabilities(uceCapabilityInfo));
                        else
                           Log.e(LOG_TAG,"onUceConfigStatusReceived received presenceStatus: "+
                     String.valueOf(uceCapabilityInfo.isPresenceEnabled) + "optionsStatus: "
                     + String.valueOf(uceCapabilityInfo.isOptionsEnabled));
                    }
            );
           }
           else
           {
                    Log.e(LOG_TAG,"getUceExecutor null onUceConfigStatusReceived received presenceStatus: "+
                     String.valueOf(uceCapabilityInfo.isPresenceEnabled) + "optionsStatus: "
                     + String.valueOf(uceCapabilityInfo.isOptionsEnabled));
           }

        }

        @Override
        public void onRcsServiceStatusUpdate(boolean b) throws RemoteException {
            if(b && acsQueryListener != null){
                acsQueryListener.onRcsInit();
            }
            if(getRcsExecutor() != null){
            getRcsExecutor().execute(
                    () -> {
                        if(rcsCbListener != null)
                        rcsCbListener.onRcsStatusReceived(b);
                        else
                           Log.e(LOG_TAG,"onRcsStatusReceived received rcsStatus: "+
                     String.valueOf(b) );
                    }
            );
        }
           else
           {
                    Log.e(LOG_TAG,"onRcsStatusReceived received rcsStatus: "+
                     String.valueOf(b) );
           }
        }

        @Override
        public void onAutoConfigErrorSipResponse(AutoConfigResponse autoConfigResponse) throws RemoteException {
            if(configCbListener!= null){
              configCbListener.handleAutoConfigErrorCb(autoConfigResponse);
            }
        }

        @Override
         public void onGetUpdatedSettings(vendor.qti.ims.configservice.V1_0.SettingsData settingsData) throws RemoteException {

         //v1.0 callback
         if(configCbListener!= null){
            if(settingsData != null){
              if(settingsData.settingsId == SettingsId.IMS_SERVICE_ENABLE_CONFIG)
              {
                  Log.e(LOG_TAG,"onGetUpdatedSettings received settingsData IMS_SERVICE_ENABLE_CONFIG: " + String.valueOf(settingsData));
                  //return;
              }
              else if(settingsData.settingsId == SettingsId.USER_AGENT_STRING){
                  Log.e(LOG_TAG,"onGetUpdatedSettings received settingsData USER_AGENT_STRING: " + String.valueOf(settingsData));
                  setUserAgentValue(settingsData.settingsValues);
              }
              //handle other settings
            }
            configCbListener.handleGetUpdatedSettingsCb(settingsData);
         }
        }
    };

    private class ConfigListener extends vendor.qti.ims.configservice.V1_1.IConfigServiceListener.Stub {

        @Override
        public void onCommandStatus(int status, int userData) throws RemoteException {
        // v1.0 callback
        if(userData == mUserData && configCbListener!= null)
               configCbListener.notifyCommandStatus(status,userData);
        else
               Log.d(LOG_TAG,"onCommandStatus from diff client, userdata: "+ String.valueOf(userData));
        }

        @Override
        public void onGetSettingsResponse(int status, vendor.qti.ims.configservice.V1_0.SettingsData settingsData, int userData) throws RemoteException {
            //v1.0 callback
            if(settingsData != null){
                 if(settingsData.settingsId == SettingsId.USER_AGENT_STRING){
                  Log.e(LOG_TAG,"onGetSettingsResponse received settingsData USER_AGENT_STRING: " + String.valueOf(settingsData));
                  setUserAgentValue(settingsData.settingsValues);
              }
              //handle other settings
            }
            if(userData == mUserData && configCbListener!= null )
                configCbListener.handleGetSettingsResp(status,settingsData,userData);
            else
                Log.d(LOG_TAG,"onGetSettingsResponse from diff client, userdata: "+ String.valueOf(userData));
            }

        @Override
        public void onAutoConfigurationReceived(AutoConfig autoConfig) throws RemoteException {
            if(configCbListener!= null)
            configCbListener.handleAutoConfigReceived(autoConfig);
        }

        @Override
        public void onReconfigNeeded() throws RemoteException {
         //ModemSSR
          Log.d(LOG_TAG,"onReconfigNeeded v1_1 received");
          isModemQmiUp = true;
          /* calling the init values when QMI service is up */
          if(initQmiServicesQueryListener != null)
             initQmiServicesQueryListener.onQmiServiceUp();
        }

        @Override
        public void onTokenFetchRequest(int i, int i1, int i2) throws RemoteException {
          Log.d(LOG_TAG,"onTokenFetchRequest received ");
          /* TODO check here if we want to store them, as service loads later */
        }

        @Override
        public void onUceStatusUpdate(vendor.qti.ims.configservice.V1_0.UceCapabilityInfo uceCapabilityInfo) throws RemoteException {
        Log.d(LOG_TAG,"onUceStatusUpdate received ");
           //configCbListener.handleUceStatusChange(getUceExecutor(),getRcsCapabilities(uceCapabilityInfo),uceCbListener);
           if(getUceExecutor() != null){
            getUceExecutor().execute(
                    () -> {
                        if(uceCbListener != null)
                        uceCbListener.onUceConfigStatusReceived(getRcsCapabilities(uceCapabilityInfo));
                        else
                           Log.e(LOG_TAG,"onUceConfigStatusReceived received presenceStatus: "+
                     String.valueOf(uceCapabilityInfo.isPresenceEnabled) + "optionsStatus: "
                     + String.valueOf(uceCapabilityInfo.isOptionsEnabled));
                    }
            );
           }
           else
           {
                    Log.e(LOG_TAG,"getUceExecutor null onUceConfigStatusReceived received presenceStatus: "+
                     String.valueOf(uceCapabilityInfo.isPresenceEnabled) + "optionsStatus: "
                     + String.valueOf(uceCapabilityInfo.isOptionsEnabled));
           }

        }

        @Override
        public void onRcsServiceStatusUpdate(boolean b) throws RemoteException {
            if(b && acsQueryListener != null){
                acsQueryListener.onRcsInit();
            }
            if(getRcsExecutor() != null){
            getRcsExecutor().execute(
                    () -> {
                        if(rcsCbListener != null)
                        rcsCbListener.onRcsStatusReceived(b);
                        else
                           Log.e(LOG_TAG,"onRcsStatusReceived received rcsStatus: "+
                     String.valueOf(b) );
                    }
            );
        }
           else
           {
                    Log.e(LOG_TAG,"onRcsStatusReceived received rcsStatus: "+
                     String.valueOf(b) );
           }
        }

        @Override
        public void onAutoConfigErrorSipResponse(AutoConfigResponse autoConfigResponse) throws RemoteException {
            if(configCbListener!= null){
            configCbListener.handleAutoConfigErrorCb(autoConfigResponse);
        }
        }

        @Override
         public void onGetUpdatedSettings(vendor.qti.ims.configservice.V1_0.SettingsData settingsData) throws RemoteException {

         //v1.0 callback
         if(configCbListener!= null){
            if(settingsData != null){
              if(settingsData.settingsId == SettingsId.IMS_SERVICE_ENABLE_CONFIG)
              {
                  Log.e(LOG_TAG,"onGetUpdatedSettings received settingsData IMS_SERVICE_ENABLE_CONFIG: " + String.valueOf(settingsData));
                  //return;
              }
              else if(settingsData.settingsId == SettingsId.USER_AGENT_STRING){
                  Log.e(LOG_TAG,"onGetUpdatedSettings received settingsData USER_AGENT_STRING: " + String.valueOf(settingsData));
                  setUserAgentValue(settingsData.settingsValues);
              }
              //handle other settings
            }
            configCbListener.handleGetUpdatedSettingsCb(settingsData);
         }
        }

        @Override
        public void onCommandStatus_1_1(int status, int userData) throws RemoteException {
             if(userData == mUserData && configCbListener!= null)
               configCbListener.notifyCommandStatus(status,userData);
             else
                 Log.d(LOG_TAG,"onCommandStatus_1_1 from diff client, userdata: "+ String.valueOf(userData));
        }

         @Override
        public void onGetSettingsResponse_1_1(int status, SettingsData settingsData, int userData) throws RemoteException {

            if(settingsData != null){
                 if(settingsData.settingsId == SettingsId.USER_AGENT_STRING){
                  Log.e(LOG_TAG,"onGetSettingsResponse_1_1 received settingsData USER_AGENT_STRING: " + String.valueOf(settingsData));
                  setUserAgentValue(settingsData.settingsValues);
              }
              //handle other settings
            }

            if(userData == mUserData && configCbListener!= null )
                configCbListener.handleGetSettingsResp(status,settingsData,userData);
            else
                Log.d(LOG_TAG,"onGetSettingsResponse_1_1 from diff client, userdata: "+ String.valueOf(userData));
        }

        @Override
        public void onGetUpdatedSettings_1_1(SettingsData settingsData) throws RemoteException {
            if(configCbListener!= null){
            if(settingsData != null){
              if(settingsData.settingsId == SettingsId.IMS_SERVICE_ENABLE_CONFIG)
              {
                  Log.e(LOG_TAG,"onGetUpdatedSettings_1_1 received settingsData IMS_SERVICE_ENABLE_CONFIG: " + String.valueOf(settingsData));
                  //return;
              }
              else if(settingsData.settingsId == SettingsId.USER_AGENT_STRING){
                  Log.e(LOG_TAG,"onGetUpdatedSettings_1_1 received settingsData USER_AGENT_STRING: " + String.valueOf(settingsData));
                  setUserAgentValue(settingsData.settingsValues);
              }
              //handle other settings
            }
            configCbListener.handleGetUpdatedSettingsCb(settingsData);
         }
        }
    };


    public void registerForSipStatusOnModem(Executor sipExec, ImsConfigServiceImpl.SipTransportCapUpdateCallback sipCb) {
      setRcsExecutor(sipExec);
      setRcsCbListener(sipCb);
      if(isModemQmiUp)
        getRcsStatus(mIsRcsEnabled);
      //update siptransport with userAgent value if already received
      notifyUserAgentStatusToSipTransport();
    }

    public void updateUceStatusOnModem(Executor uceExec, RcsFeature.RcsImsCapabilities caps, ImsConfigServiceImpl.UceCapUpdateCallback uceCb) {
      setUceExecutor(uceExec);
      setUceCbListener(uceCb);
      cacheUceCap(caps);
      if(isModemQmiUp)
        getUceStatus(caps);
    }

    private void cacheUceCap(RcsFeature.RcsImsCapabilities caps){
        this.uceCaps = caps;
    }

    public void getUceStatus(RcsFeature.RcsImsCapabilities caps) {

        IConfigService.getUceStatusCallback configCb = new IConfigService.getUceStatusCallback() {
            @Override
            public void onValues(int status, UceCapabilityInfo uceCapabilityInfo) {
                //post to Uce Thread
                Log.e(LOG_TAG,"IConfigService.getUceStatusCallback:  received rcsStatus: " + status);
                    /*TODO: print ThreadId in onValues */
                    if(status == RequestStatus.OK){
                    RcsFeature.RcsImsCapabilities localCaps = getRcsCapabilities(uceCapabilityInfo);
                     Log.d(LOG_TAG,"cap from UCE module presence : " + String.valueOf(caps.isCapable(CAPABILITY_TYPE_PRESENCE_UCE)) + " options: "+ caps.isCapable(CAPABILITY_TYPE_OPTIONS_UCE));
                    if(caps.isCapable(CAPABILITY_TYPE_PRESENCE_UCE) == localCaps.isCapable(CAPABILITY_TYPE_PRESENCE_UCE)
                        && caps.isCapable(CAPABILITY_TYPE_OPTIONS_UCE) == localCaps.isCapable(CAPABILITY_TYPE_OPTIONS_UCE)){
                         Log.d(LOG_TAG,"calling onUceConfigStatusReceived");
                         uceEx.execute( () -> {
                                       if(uceCbListener != null)
                                        uceCbListener.onUceConfigStatusReceived(localCaps);
                                } );
                    }
                    else{
                    //     //call set if anything mismatch frm modem and java
                          Log.d(LOG_TAG,"calling setUceStatus");
                        setUceStatus(caps.isCapable(CAPABILITY_TYPE_PRESENCE_UCE),caps.isCapable(CAPABILITY_TYPE_OPTIONS_UCE));
                            }

                } else if(status == RequestStatus.IN_PROGRESS){
                      /* Qmi service not up on modem, wait for onReconfigneeded() to call setUceStatus() */
                        }
            }
        };
        if(mHidlConfigService == null)
          Log.e(LOG_TAG, "mHidlConfigService is null");
        if(mHidlConfigService != null) {
        try {
            Log.e(LOG_TAG, "starting to query UCE Status");
            mHidlConfigService.getUceStatus(configCb);
        } catch (RemoteException e) {
            Log.e(LOG_TAG, "Unable to query UCE Status");
        }
    }
        else if(mHidlConfigService_1_0 != null){
        try {
            Log.e(LOG_TAG, "starting to query UCE Status v1_0 service");
            mHidlConfigService_1_0.getUceStatus(configCb);
        } catch (RemoteException e) {
            Log.e(LOG_TAG, "Unable to query UCE Status");
        }

        }
    }

    public void setUceCbListener(ImsConfigServiceImpl.UceCapUpdateCallback l) {
        this.uceCbListener = l;
    }

    private RcsFeature.RcsImsCapabilities getRcsCapabilities(UceCapabilityInfo uceCapabilityInfo) {
        RcsFeature.RcsImsCapabilities capabilities = new RcsFeature.RcsImsCapabilities(CAPABILITY_TYPE_NONE);
        int uceCaps = CAPABILITY_TYPE_NONE;
        if(uceCapabilityInfo.isOptionsEnabled) {
            uceCaps = uceCaps|CAPABILITY_TYPE_OPTIONS_UCE;
            Log.d(LOG_TAG,"getRcsCapabilities isOptionsEnabled");
        }
        if(uceCapabilityInfo.isPresenceEnabled) {
            uceCaps = uceCaps|CAPABILITY_TYPE_PRESENCE_UCE;
            Log.d(LOG_TAG,"getRcsCapabilities isPresenceEnabled");
        }
        capabilities.addCapabilities(uceCaps);
        return capabilities;
    }


    public void getRcsStatus(boolean isRcsEnabled){

        IConfigService.getRcsServiceStatusCallback rcsCb = new IConfigService.getRcsServiceStatusCallback() {
            //post rcsservice status to SipTransport thread
            @Override
            public void onValues(int status, boolean b) {

                if(status == RequestStatus.OK){
                    //if rcs status is enabled, query for ACS
                    if(b && acsQueryListener != null){
                        acsQueryListener.onRcsInit();
                    }
                    if(b == isRcsEnabled){
               rcsEx.execute(
                       () -> {
                                    if(rcsCbListener != null){
                                        Log.d(LOG_TAG,"calling onRcsStatusReceived status: " + String.valueOf(b));
                                        rcsCbListener.onRcsStatusReceived(b);
                        }
                        }
                        );
            }
                    else{
                        //call set if anything mismatch frm modem and java
                        Log.d(LOG_TAG,"calling setRcsStatus status" + String.valueOf(isRcsEnabled));
                        setRcsStatus(isRcsEnabled);
                    }
                } else if(status == RequestStatus.IN_PROGRESS){
                    /* Qmi service not up on modem, wait for onReconfigneeded() to call setUceStatus() */
                }
            }
        };

        if(mHidlConfigService != null){
        try {
            mHidlConfigService.getRcsServiceStatus(rcsCb);
        }
        catch (RemoteException e){
            Log.e(LOG_TAG,"Unable to fetch RCS Service Status");
        }
    }
        else if(mHidlConfigService_1_0 != null){
        try {
            mHidlConfigService_1_0.getRcsServiceStatus(rcsCb);
        }
        catch (RemoteException e){
            Log.e(LOG_TAG,"Unable to fetch RCS Service Status _1_0");
        }

        }
    }

    public void setRcsCbListener(ImsConfigServiceImpl.SipTransportCapUpdateCallback l) {
        this.rcsCbListener = l;
    }

    public int setConfig(byte[] data, boolean isCompressed){
        ConfigData configHidlData = new ConfigData();
        int status = RequestStatus.FAIL;
        configHidlData.isCompressed = isCompressed;
        for(Byte b : data) {
            configHidlData.config.add(b);
        }
        //TODO check the 2nd condition
        if(cachedConfig == null /*|| !Arrays.equals(cachedConfig,data) */){
            cachedConfig = data.clone();
            isCachedConfigCompressed = isCompressed;
        }
        // if iscompressed, check if we have to compress and send the xml or not TODO */
        if(mHidlConfigService != null){
        try{
            Log.d(LOG_TAG,"calling setconfig");
            //custom userData to check the response properly
            status = mHidlConfigService.setConfig(configHidlData, 5678);
        }
        catch(RemoteException e){
            Log.d(LOG_TAG,e.toString());
        }
        }
        else if(mHidlConfigService_1_0 != null){
        try{
            Log.d(LOG_TAG,"calling setconfig");
            //custom userData to check the response properly
            status = mHidlConfigService_1_0.setConfig(configHidlData, 5678);
        }
        catch(RemoteException e){
            Log.d(LOG_TAG,e.toString());
        }

        }
        return status;
    }

    private int sendConfigSettings(int id, vendor.qti.ims.configservice.V1_0.SettingsValues values){
        int status = RequestStatus.FAIL;
        vendor.qti.ims.configservice.V1_0.SettingsData settingsData = new vendor.qti.ims.configservice.V1_0.SettingsData();
        vendor.qti.ims.configservice.V1_0.SettingsValues valuesList = new vendor.qti.ims.configservice.V1_0.SettingsValues();
        valuesList.intData = new ArrayList<>(values.intData);
        valuesList.stringData = new ArrayList<>(values.stringData);
        valuesList.boolData = new ArrayList<>(values.boolData);
        settingsData.settingsValues = valuesList;
        settingsData.settingsId = id;

        if(mHidlConfigService_1_0 != null){
            try {
              Log.d(LOG_TAG,"calling sendConfigSettings setsettingsValue " + values.toString());
              status = mHidlConfigService_1_0.setSettingsValue(settingsData,mUserData);
            }
            catch (RemoteException e) {
              Log.d(LOG_TAG,e.toString());
            }
        } else {
            Log.e(LOG_TAG," sendConfigSettings : mHidlConfigService_1_0 not initialized");
        }
        return status;
    }

    private int sendConfigSettings(int id, SettingsValues values){
        int status = RequestStatus.FAIL;
        SettingsData settingsData = new SettingsData();
        SettingsValues valuesList = new SettingsValues();
        valuesList.intData = new ArrayList<>(values.intData);
        valuesList.stringData = new ArrayList<>(values.stringData);
        valuesList.boolData = new ArrayList<>(values.boolData);
        settingsData.settingsValues = valuesList;
        settingsData.settingsId = id;

        if(mHidlConfigService != null){
            try {
                Log.d(LOG_TAG,"calling sendConfigSettings setsettingsValue_1_1 " + values.toString());
                status = mHidlConfigService.setSettingsValue_1_1(settingsData,mUserData);
            }
            catch (RemoteException e) {
                Log.d(LOG_TAG,e.toString());
            }
        } else {
            Log.e(LOG_TAG," sendConfigSettings : mHidlConfigService not initialized");
        }

        return status;
    }

    public void registerForSettingsChange()
    {
      if(mHidlConfigService != null){
      try {
       if(isModemQmiUp && !isRegForInd){
             Log.d(LOG_TAG,"registerForSettingsChange ");
            mHidlConfigService.registerForSettingsChange(mUserData);
               isRegForInd = true;
       }
      } catch(RemoteException e) {
        Log.d(LOG_TAG,e.toString());
      }
      }
      else if(mHidlConfigService_1_0 != null){
      try {
       if(isModemQmiUp && !isRegForInd){
             Log.d(LOG_TAG,"registerForSettingsChange mHidlConfigService_1_0 ");
            mHidlConfigService_1_0.registerForSettingsChange(mUserData);
               isRegForInd = true;
       }
      } catch(RemoteException e) {
        Log.d(LOG_TAG,e.toString());
      }

      }
    }
    public void queryAcsConfiguration()
    {
      if(mHidlConfigService != null){
      try {
            Log.d(LOG_TAG,"calling getAcsConfiguration ");
            mHidlConfigService.getAcsConfiguration(mUserData);
        } catch (RemoteException e) {
            Log.d(LOG_TAG,e.toString());
        }
      }
      else if(mHidlConfigService_1_0 != null) {
      try {
            Log.d(LOG_TAG,"calling getAcsConfiguration mHidlConfigService_1_0 ");
            mHidlConfigService_1_0.getAcsConfiguration(mUserData);
        } catch (RemoteException e) {
            Log.d(LOG_TAG,e.toString());
        }

      }
    }

    public void deregisterForSettingsChange()
    {
      if(mHidlConfigService != null){
      try {
        if(isModemQmiUp && isRegForInd){
        Log.d(LOG_TAG,"deregisterForSettingsChange ");
          mHidlConfigService.deregisterForSettingsChange(mUserData);
          isRegForInd=false;
        }
      } catch(RemoteException e) {
          Log.d(LOG_TAG,e.toString());
      }
    }
      else if(mHidlConfigService_1_0 != null) {
      try {
        if(isModemQmiUp && isRegForInd){
          Log.d(LOG_TAG,"deregisterForSettingsChange mHidlConfigService_1_0 ");
          mHidlConfigService_1_0.deregisterForSettingsChange(mUserData);
          isRegForInd=false;
        }
      } catch(RemoteException e) {
          Log.d(LOG_TAG,e.toString());
      }

      }
    }

    public void registerAutoConfig(ImsConfigServiceImpl.QueryAutoConfigListener acsQueryListener){
       this.acsQueryListener = acsQueryListener;
    }

    public void registerQmiServiceUp(ImsConfigServiceImpl.InitQmiRequestsListener initQmiServicesQueryListener){
       this.initQmiServicesQueryListener = initQmiServicesQueryListener;

       if(!isModemQmiUp) {
          Log.d(LOG_TAG,"registerQmiServiceUp done, wait for qmiService up as currently modem is not up");
          return;
       }
       initQmiServicesQueryListener.onQmiServiceUp();
    }

    public void queryInitCachedRequests(){
        Log.d(LOG_TAG,"queryInitCachedRequests");
        Log.d(LOG_TAG,"isModemQmiUp: " + isModemQmiUp);
        registerForSettingsChange();

       // send cached config to modem if client already requested
        if(cachedConfig != null)
          setConfig(cachedConfig,isCachedConfigCompressed);

        if(uceCaps != null)
          getUceStatus(uceCaps);

        Log.d(LOG_TAG,"calling setRcsStatus ");
        getRcsStatus(mIsRcsEnabled);

        // send cached rcsclientconfiguration to modem if client already requested
        if(cacheRcsClientConfig != null)
          sendRcsClientConfig(cacheRcsClientConfig);

        queryInitBootupRequests();
    }

    //to be called on initialization or when imsrcsservice restarts
    public void queryInitBootupRequests(){
      Log.d(LOG_TAG,"queryInitBootupRequests");
        if(mUserAgentValue == null){
            getSettingsValue(SettingsId.USER_AGENT_STRING);
        }
    }

    public void getSettingsValue(int settingsId){
      if(mHidlConfigService != null){
      try {
            Log.d(LOG_TAG,"calling getSettingsValue ");
            mHidlConfigService.getSettingsValue(settingsId,mUserData);
        } catch (RemoteException e) {
            Log.d(LOG_TAG,e.toString());
        }
      }
      else if(mHidlConfigService_1_0 != null) {
      try {
            Log.d(LOG_TAG,"calling getSettingsValue mHidlConfigService_1_0 ");
            mHidlConfigService_1_0.getSettingsValue(settingsId,mUserData);
        } catch (RemoteException e) {
            Log.d(LOG_TAG,e.toString());
        }

      }
    }

}
