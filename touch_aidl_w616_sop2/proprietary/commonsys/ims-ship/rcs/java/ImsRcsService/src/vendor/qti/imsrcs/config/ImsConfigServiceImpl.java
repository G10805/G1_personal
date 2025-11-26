/**********************************************************************
 * Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/


package vendor.qti.imsrcs.config;

import android.os.RemoteException;
import android.telephony.ims.aidl.IImsRegistration;
import android.telephony.ims.aidl.IImsServiceController;
import android.telephony.ims.aidl.IImsServiceControllerListener;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.RcsFeature;
import android.telephony.ims.stub.ImsConfigImplBase;
import android.util.Log;
import android.util.SparseArray;

import com.android.ims.internal.IImsFeatureStatusCallback;
import com.android.internal.annotations.VisibleForTesting;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.HashMap;
import java.util.Map;

import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.RcsFeature;

import java.util.ArrayList;
import java.util.List;
//import vendor.qti.ims.SparseArray;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.RcsClientConfiguration;
import android.telephony.ims.RcsConfig;
import android.telephony.ims.aidl.IImsConfig;
import android.telephony.ims.feature.RcsFeature;
import android.telephony.ims.stub.ImsConfigImplBase;
import android.util.Log;

import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;

import vendor.qti.ims.configservice.V1_0.AutoConfigTriggerReason;
import vendor.qti.imsrcs.config.ImsConfigServiceWrapper;
import vendor.qti.imsrcs.ImsRcsServiceMgr;


//import vendor.qti.imsrcs.uce;
//import vendor.qti.imsrcs.siptransport;

public class ImsConfigServiceImpl extends ImsConfigImplBase {

    private final String LOG_TAG = "ImsConfigServiceImpl";
    private static final int INVALID_SLOT_ID = -1;
     private static final String VERSION_NAME = "client_version_name";
    private static final String VERSION_CODE = "client_version_code";
    private PersistableBundle mBundle;
    private CarrierConfigManager mCarrierConfigManager;
    private Context mContext;
    private int mSlotId;
    Executor mConfigExec;
    ImsConfigServiceWrapper mHidlImsConfig;
    ConfigCbListener cb;
    private int errorCode;
    private String errorString;
    ProvisioningManager pm;
    RcsClientConfiguration rcsClientConfig = null;
    private boolean isDmaChange = false;
    private String mPackageVersion;
    private long mVersionCode;
    private boolean isDeviceUpgrade = false;
    private Executor uceExec;
    private Executor rcsExec;
    private ImsConfigServiceImpl.UceCapUpdateCallback uceCb = null;
    private ImsConfigServiceImpl.SipTransportCapUpdateCallback rcsCb = null;
    private RcsFeature.RcsImsCapabilities uceCap = null;
    private boolean isRegForInd = false;


    public abstract static class SipTransportCapUpdateCallback {
        public abstract void onRcsStatusReceived(boolean isRcsEnabled);
        public abstract void onUserAgentStringReceived(String userAgent);
    };

    public abstract static class UceCapUpdateCallback {
        public abstract void onUceConfigStatusReceived(RcsFeature.RcsImsCapabilities cap);
    }

    public abstract static class QueryAcsCallback {
        public abstract void onRcsInit();
    }

    public abstract static class QueryInitQmiServiceCallback {
        public abstract void onQmiServiceUp();
    }

    public ImsConfigServiceImpl(Context context) {
        super(context);
    }

    public ImsConfigServiceImpl() {
        super();
    }

    public ImsConfigServiceImpl(int slotId, Context context) {
        super(context);
        this.mSlotId = slotId;
        mConfigExec = new ScheduledThreadPoolExecutor(1);
        this.mContext = context;
        pm = ProvisioningManager.createForSubscriptionId(slotId);
        mBundle = new PersistableBundle();
        initConfigWrapper();
    }

    public void initConfigWrapper() {
        mConfigExec.execute(
                () -> {
                    cb = new ConfigCbListener(mConfigExec);
                    mHidlImsConfig = ImsRcsServiceMgr.getInstance().getConfigService(mSlotId);
                    if(mHidlImsConfig != null){
                      mHidlImsConfig.setConfigCbListener(cb);
                      Log.d(LOG_TAG,"initialized configwrapper");
                      mHidlImsConfig.registerAutoConfig(new QueryAutoConfigListener());
                      mHidlImsConfig.registerQmiServiceUp(new InitQmiRequestsListener());
                      mHidlImsConfig.queryInitBootupRequests();
                      /* calling this on re-init of Wrapper/ imsd restart */
                      if(uceCb != null )
                        mHidlImsConfig.updateUceStatusOnModem(uceExec,uceCap,uceCb);
                      if(rcsCb != null)
                        mHidlImsConfig.registerForSipStatusOnModem(rcsExec,rcsCb);
                    }
                    else{
                      Log.d(LOG_TAG,"mHidlImsConfig null:initConfigWrapper");
                    }
                }
        );
    }

    public void deInit(){

    mConfigExec.execute(
                () -> {
                     Log.d(LOG_TAG,"deInit");
                     mHidlImsConfig.deregisterForSettingsChange();
                     cb = null;
                     mHidlImsConfig.setConfigCbListener(null);
                     mHidlImsConfig.setHidlConfigService(null);
                     mHidlImsConfig.setHidlConfigService_1_0(null);
                     mHidlImsConfig = null;
           }
        );
    //mHidlImsConfig = null;
    }

    @Override
    public void notifyRcsAutoConfigurationReceived(byte[] config, boolean isCompressed) {
            mConfigExec.execute(
                    () -> {
                        if(mHidlImsConfig != null){
                           Log.d(LOG_TAG,"notifyRcsAutoConfigurationReceived");
                        mHidlImsConfig.setConfig(config, isCompressed);
                        }
                        else
                           Log.d(LOG_TAG,"mHidlImsConfig null:notifyRcsAutoConfigurationReceived");
                    }
            );

    }

    /**
     * would be called on dmachange or client/fw restart
     */
    @Override
    public void notifyRcsAutoConfigurationRemoved() {
        mHidlImsConfig.clearConfigCache();
    }

    @Override
    public int setConfig(int item, int value) {
        return super.setConfig(item, value);
    }

    @Override
    public int setConfig(int item, String value) {
        return super.setConfig(item, value);
    }

    @Override
    public int getConfigInt(int item) {
        return super.getConfigInt(item);
    }

    @Override
    public String getConfigString(int item) {
        return super.getConfigString(item);
    }

    @Override
    public void updateImsCarrierConfigs(PersistableBundle bundle) {
        super.updateImsCarrierConfigs(bundle);
    }


    @Override
    public IImsConfig getIImsConfig() {
        return super.getIImsConfig();
    }



    /**
     * When the default messaging application specific parameters change
     * and are sent, it means clientchange
     * @param rcc
     * @return
     */
    @Override
    public void setRcsClientConfiguration(RcsClientConfiguration rcc) {
        //stores the client param
        //final int status = -1;
        if(isClientConfigChange(rcc)){
       // Log.d(LOG_TAG, "isClientConfigChange");
             isDmaChange = true;
        }
        mConfigExec.execute(
                ()-> {
                    if(mHidlImsConfig != null){
                      mHidlImsConfig.sendRcsClientConfig(rcc);
                      Log.d(LOG_TAG,"sendRcsClientConfig");
                    }
                    else{
                      Log.d(LOG_TAG,"mHidlImsConfig null:setRcsClientConfiguration");
                    }
                }
        );
    }

    @Override
    public void triggerAutoConfiguration() {
        int reasonCode;
        reasonCode = getAcsTriggerReason();
        mConfigExec.execute(
                () -> {
                if(mHidlImsConfig != null){
                    mHidlImsConfig.triggerAcsRequest(reasonCode);
                }
                else{
                      Log.d(LOG_TAG,"mHidlImsConfig null:triggerAutoConfiguration");
                    }

                }
        );

                }




    //stores clientConfig if its null and compares if it has changed
    //TODO: Check if its needed to be persistent.
    private boolean isClientConfigChange(RcsClientConfiguration rcc) {
         Log.d(LOG_TAG, "isClientConfigChange");
            if (rcsClientConfig == null || (rcc.getClientVendor().equals(rcsClientConfig.getClientVendor())||
                    rcc.getClientVersion().equals(rcsClientConfig.getClientVersion()) ||
                    rcc.getRcsProfile().equals(rcsClientConfig.getRcsProfile())||
                    rcc.getRcsVersion().equals(rcsClientConfig.getRcsVersion()))) {
                rcsClientConfig = new RcsClientConfiguration(rcc.getRcsVersion(),rcc.getRcsProfile(),rcc.getClientVendor(),rcc.getClientVersion());
                return true;
            }
        return false;

    }

    private void setUceModuleParams(Executor uceExec, RcsFeature.RcsImsCapabilities cap, UceCapUpdateCallback uceCb)
    {
      this.uceExec = uceExec;
      this.uceCap = cap;
      this.uceCb = uceCb;
    }

    private void setRcsModuleParams(Executor rcsExec, SipTransportCapUpdateCallback rcsCb)
    {
      this.rcsExec=rcsExec;
      this.rcsCb = rcsCb;
    }

    public void updateUceCapability(Executor uceExec, RcsFeature.RcsImsCapabilities cap, UceCapUpdateCallback sipCb)
    {
        setUceModuleParams(uceExec,cap,sipCb);
        if(mHidlImsConfig != null) {
        mConfigExec.execute(
                () -> {
                        Log.d(LOG_TAG, "updateUceCapability called");
                        //TODO : check if getAcs to be called from here???
                        synchronized(mHidlImsConfig){
                        mHidlImsConfig.updateUceStatusOnModem(uceExec,cap,sipCb);
                        }
                }
        );
        }

    }

    public void registerForSipTransportCapability(Executor sipExec, SipTransportCapUpdateCallback sipCb)
    {
        setRcsModuleParams(sipExec,sipCb);
        if(mHidlImsConfig != null) {
        mConfigExec.execute(
                () -> {

                        Log.d(LOG_TAG, "registerForSipTransportCapability called");
                        synchronized(mHidlImsConfig){
                        mHidlImsConfig.registerForSipStatusOnModem(sipExec,sipCb);
                        }

                }
        );
            }

    }

    private int getAcsTriggerReason() {
        int reason;
        if(isDmaChange)
            reason = AutoConfigTriggerReason.AUTOCONFIG_CLIENT_CHANGE;
        else if(isClientUpgrade())
            reason = AutoConfigTriggerReason.AUTOCONFIG_DEVICE_UPGRADE;
        else if(isFactoryResetEvt())
            reason = AutoConfigTriggerReason.AUTOCONFIG_FACTORY_RESET;
        else
            reason = AutoConfigTriggerReason.AUTOCONFIG_INVALID_TOKEN;

        return reason;
    }

    private boolean isFactoryResetEvt() {

        return false;
    }

    private boolean isClientUpgrade() {

        String vName = mBundle.getString(ImsConfigServiceImpl.VERSION_NAME);
        long vCode = mBundle.getLong(ImsConfigServiceImpl.VERSION_CODE,-1);
        if(vName == null || vCode == -1){
            //first init
            mBundle.putString(ImsConfigServiceImpl.VERSION_NAME,"");
            mBundle.putLong(ImsConfigServiceImpl.VERSION_CODE,0);
            return false;
        }
        PackageInfo pInfo = null;
        try {
            pInfo = mContext.getPackageManager().getPackageInfo(mContext.getPackageName(), 0);
            String version = pInfo.versionName;
            long versionCode = pInfo.getLongVersionCode();
            /*TODO: do we need to check and compare both */
            if(!version.equals(vName) || versionCode != vCode) {
                mBundle.putString(ImsConfigServiceImpl.VERSION_NAME,version);
                mBundle.putLong(ImsConfigServiceImpl.VERSION_CODE,versionCode);
                return true;
            }
        } catch (PackageManager.NameNotFoundException nameNotFoundException) {
            nameNotFoundException.printStackTrace();
            Log.d(LOG_TAG,"Exception for device upgrade");
        }
        catch (Exception e){
            Log.d(LOG_TAG,"nullpointer ex due to: "+ e.toString());
        }

        return false;
    }

    private void clearClientUpgradeParams()
    {
        mBundle.putString(VERSION_NAME,null);
        mBundle.putLong(VERSION_CODE,-1);
    }

    public void dispose() {
        clearClientUpgradeParams();
        rcsClientConfig = null;
    }

    private class ConfigCbListener extends ImsConfigCbListener {

        public ConfigCbListener(Executor mConfigExec) {
            super(mConfigExec);

        }

        @Override
        public void onCommandStatusCb(int status, int userData) {
            Log.d(LOG_TAG,String.valueOf(status) + "userData: " + String.valueOf(userData));
            return;
        }

        @Override
        public void onAutoConfigurationReceivedCb(byte[] autoConfigXml, boolean isCompressed) {
            //notifyRcsAutoConfigurationReceived(autoConfigXml,isCompressed);
            Log.d(LOG_TAG,"passing AutoConfigurationReceived to FW: xmlSize:  "+ String.valueOf(autoConfigXml.length) + " isCompressed: " + String.valueOf(isCompressed));
            try {
                getIImsConfig().notifyRcsAutoConfigurationReceived(autoConfigXml,isCompressed);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }


        @Override
        public void onAutoConfigurationErrorReceivedCb(int errorCode, String errorStr) {
            notifyAutoConfigurationErrorReceived(errorCode,errorString);
        }

        @Override
        public void onPreConfigReceivedCb(byte[] preconfigxml) {
            notifyPreProvisioningReceived(preconfigxml);
        }

        @Override
        public void onWrapperReset() {
            ImsConfigServiceImpl.this.deInit();
        }

    }


    class QueryAutoConfigListener extends QueryAcsCallback
    {
        @Override
        public void onRcsInit(){
            if(mHidlImsConfig != null){
              mConfigExec.execute(
              () -> {
                mHidlImsConfig.queryAcsConfiguration();
              });
          }
        }
    }

    class InitQmiRequestsListener extends QueryInitQmiServiceCallback
    {
        @Override
        public void onQmiServiceUp(){
            if(mHidlImsConfig != null){
              mConfigExec.execute(
              () -> {
                mHidlImsConfig.queryInitCachedRequests();
              });
          }
        }
    }
}
