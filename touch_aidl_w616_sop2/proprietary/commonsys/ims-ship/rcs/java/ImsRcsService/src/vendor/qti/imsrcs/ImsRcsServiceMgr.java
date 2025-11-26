/**********************************************************************
 * Copyright (c) 2021-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsrcs;

import android.os.IHwBinder;
import android.os.RemoteException;
import android.telephony.ims.stub.CapabilityExchangeEventListener;
import android.util.Log;
import android.hidl.manager.V1_2.IServiceManager;

import vendor.qti.imsrcs.config.ImsConfigServiceWrapper;
import vendor.qti.imsrcs.siptransport.hidl.SipTransportServiceWrapper;
import vendor.qti.imsrcs.uce.hidl.OptionsServiceWrapper;
import vendor.qti.imsrcs.uce.hidl.PresenceServiceWrapper;
import vendor.qti.imsrcs.uce.ImsRcsCapabilityExchangeImpl;

import java.util.NoSuchElementException;
import java.util.concurrent.Executor;
import java.util.List;

import vendor.qti.ims.factory.V2_2.IImsFactory;
import vendor.qti.ims.rcsuce.V1_2.IPresenceService;
import vendor.qti.ims.configservice.V1_1.IConfigService;
import vendor.qti.ims.rcsuce.V1_0.IOptionsService;
import vendor.qti.ims.rcssip.V1_2.ISipTransportService;
import vendor.qti.imsrcs.config.ImsConfigServiceImpl;

//singleton
public class ImsRcsServiceMgr {
    final String LOG_TAG = "ImsRcsServiceMgr";
    static IImsFactory mImsFactoryInstance = null;
    static vendor.qti.ims.factory.V2_1.IImsFactory mImsFactoryInstance_2_1 = null;
    static vendor.qti.ims.factory.V2_0.IImsFactory mImsFactoryInstance_2_0 = null;
    static boolean mImsFactoryInitialized = false;
    PresenceServiceWrapper[] mPresenceService;
    OptionsServiceWrapper[] mOptionsService;
    ImsConfigServiceWrapper[] mConfigService;
    SipTransportServiceWrapper[] mSipTransportService;
    ImsRcsCapabilityExchangeImpl[] mRcsCapExchanges;
    private static ImsRcsServiceMgr mMgr;
    int MAX_RETRY_COUNT = 10;
    int REINTIALIZE_FLAG = 0;
    int[] mSipTransportSvcRetryCount;
    int[] mPresenceSvcReinitFlag;
    int[] mPresenceSvcRetryCount;
    boolean[] mIsSipTransportInited;

    private ImsRcsServiceMgr() {
        mSipTransportService = new SipTransportServiceWrapper[ImsRcsService.MAX_SLOTS];
        mPresenceService = new PresenceServiceWrapper[ImsRcsService.MAX_SLOTS];
        mOptionsService = new OptionsServiceWrapper[ImsRcsService.MAX_SLOTS];
        mSipTransportSvcRetryCount = new int[ImsRcsService.MAX_SLOTS];
        mPresenceSvcReinitFlag = new int[ImsRcsService.MAX_SLOTS];
        mPresenceSvcRetryCount = new int[ImsRcsService.MAX_SLOTS];
        mIsSipTransportInited = new boolean[ImsRcsService.MAX_SLOTS];
        mRcsCapExchanges = new ImsRcsCapabilityExchangeImpl[ImsRcsService.MAX_SLOTS];
    }

    synchronized public static ImsRcsServiceMgr getInstance() {
        if(mMgr == null) {
          mMgr = new ImsRcsServiceMgr();
        }
        return mMgr;
    }

    public void dispose() {
        Log.d(LOG_TAG, "dispose()");
        if(mConfigService != null) {
            for(ImsConfigServiceWrapper icsWrapper : mConfigService) {
                if(icsWrapper != null) {
                    icsWrapper.setConfigCbListener(null);
                    icsWrapper.setUceExecutor(null);
                    icsWrapper.setUceCbListener(null);
                } else {
                    Log.d(LOG_TAG, "ImsConfigServiceWrapper is null");
                }
            }
        } else {
            Log.d(LOG_TAG, "mConfigService list is null");
        }

        for(SipTransportServiceWrapper stsWrapper : mSipTransportService) {
            if(stsWrapper != null) {
                stsWrapper.clearSipTransportService();
            } else {
                Log.d(LOG_TAG, "SipTransportServiceWrapper is null");
            }
        }
    }

    public boolean Initialize() {
        if (mImsFactoryInitialized) {
            return true;
        }

        //Initialize the arrays of Hidl objects required for this service
        Log.d(LOG_TAG, "initialize()");
        mConfigService = new ImsConfigServiceWrapper[ImsRcsService.MAX_SLOTS];

        try {
            if (isInterfaceListedInManifest("vendor.qti.ims.factory@2.2::IImsFactory")) {
                mImsFactoryInstance = IImsFactory.getService("default",true);
                Log.d(LOG_TAG, "IImsFactory.getService");
                if(mImsFactoryInstance != null) {
                  mImsFactoryInitialized = true;
                }
            }
        } catch (RemoteException | NoSuchElementException e) {
            mImsFactoryInstance = null;
            Log.e(LOG_TAG, "Unable to bind to IImsFactory instance");
        }
        if (!mImsFactoryInitialized) {
            if (isInterfaceListedInManifest("vendor.qti.ims.factory@2.1::IImsFactory")) {
                if (fallbackToFactory2_1())
                    mImsFactoryInitialized = true;
            }
        }
        if(!mImsFactoryInitialized) {
            if(isInterfaceListedInManifest("vendor.qti.ims.factor@2.0::IImsFactory")) {
                if(fallbackToFactory2_0())
                    mImsFactoryInitialized =true;
            }
        }
        if(!mImsFactoryInitialized)
                return false;

        try {
            if (mImsFactoryInstance != null) {
                mImsFactoryInstance.linkToDeath(new FactoryDeathRecipient(), -1);
                Log.d(LOG_TAG, "mImsFactoryInstance.linkToDeath");
            } else if (mImsFactoryInstance_2_1 != null) {
                mImsFactoryInstance_2_1.linkToDeath(new FactoryDeathRecipient(), -1);
                Log.d(LOG_TAG, "mImsFactoryInstance_2_1.linkToDeath");
            } else if (mImsFactoryInstance_2_0 != null) {
                mImsFactoryInstance_2_0.linkToDeath(new FactoryDeathRecipient(), -1);
                Log.d(LOG_TAG, "mImsFactoryInstance_2_0.linkToDeath");
            }
        } catch (RemoteException e) {
            Log.e(LOG_TAG, "Unable to bind to DeathRecipient");
        }
        return  true;
    }

    synchronized private vendor.qti.ims.factory.V2_2.IImsFactory getFactoryInstance2_2() {
        return mImsFactoryInstance;
    }

    synchronized private vendor.qti.ims.factory.V2_1.IImsFactory getFactoryInstance2_1() {
        vendor.qti.ims.factory.V2_1.IImsFactory inst = getFactoryInstance2_2();
        if (inst == null) {
            inst = mImsFactoryInstance_2_1;
        }
        return inst;
    }

    synchronized private vendor.qti.ims.factory.V2_0.IImsFactory getFactoryInstance2_0() {
        vendor.qti.ims.factory.V2_0.IImsFactory inst = getFactoryInstance2_1();
        if (inst == null) {
            inst = mImsFactoryInstance_2_0;
        }
        return inst;
    }

    public ImsRcsCapabilityExchangeImpl getRcsCapExchangeImpl(
            CapabilityExchangeEventListener l, int SlotId, Executor localexec) {
        //@NOTE: We will create new CapExImpl if fw Requests it
        mRcsCapExchanges[SlotId] = new ImsRcsCapabilityExchangeImpl(localexec, l, SlotId);
        return mRcsCapExchanges[SlotId];
    }
    public ImsConfigServiceWrapper getConfigService(int slotId) {
        if((mImsFactoryInstance != null || mImsFactoryInstance_2_1 != null) && mConfigService[slotId] == null) {
            //Create the hidl service object only if needed
            mConfigService[slotId] = new ImsConfigServiceWrapper();

            IImsFactory.createConfigServiceCallback hidl_config_cb = new IImsFactory.createConfigServiceCallback() {
                @Override
                public void onValues(int status, vendor.qti.ims.configservice.V1_0.IConfigService iImsConfigService) {
                    Log.d(LOG_TAG + " V1.0 IConfigService status: ",IImsFactory.StatusCode.toString(status));
                    if(status == IImsFactory.StatusCode.OK) {
                        mConfigService[slotId].setHidlConfigService_1_0(iImsConfigService);
                    }
                    else{
                        // handle scenario when iconfigservice creation fails
                    }
                }
            };

            IImsFactory.createConfigService_1_1Callback _hidl_cb = new IImsFactory.createConfigService_1_1Callback() {
                @Override
                public void onValues(int status, IConfigService iImsConfigService) {
                    Log.d(LOG_TAG + " V1.1 IConfigService status: ",IImsFactory.StatusCode.toString(status));
                    if(status == IImsFactory.StatusCode.OK) {
                        mConfigService[slotId].setHidlConfigService(iImsConfigService);
                    }
                    else if(status == IImsFactory.StatusCode.NOT_SUPPORTED){
                        // query for v1.0
                        try {
                            if(mImsFactoryInstance != null) {
                                mImsFactoryInstance.createConfigService(slotId+1,
                                   mConfigService[slotId].gethidlConfigListener(), hidl_config_cb);
                            } else if(mImsFactoryInstance_2_1 != null) {
                                mImsFactoryInstance_2_1.createConfigService(slotId+1,
                                   mConfigService[slotId].gethidlConfigListener(), hidl_config_cb);
                            }
                        } catch (RemoteException | NoSuchElementException e) {
                            Log.e(LOG_TAG, "Unable to get to IConfigService instance");
                        }
                    }
                }
            };

            try {
                if(mImsFactoryInstance != null) {
                    mImsFactoryInstance.createConfigService_1_1(slotId+1,
                         mConfigService[slotId].gethidlConfigListener(), _hidl_cb);
                } else if(mImsFactoryInstance_2_1 != null) {
                    mImsFactoryInstance_2_1.createConfigService_1_1(slotId+1,
                         mConfigService[slotId].gethidlConfigListener(), _hidl_cb);
                }
            } catch (RemoteException | NoSuchElementException e) {
                Log.e(LOG_TAG, "Unable to get to IConfigService instance");
            }
        } // TODO add mImsFactoryInstance_2_1 case
        else if(mImsFactoryInstance_2_0 != null){

          mConfigService[slotId] = new ImsConfigServiceWrapper();
          mConfigService[slotId].createHidlConfigListener_1_0();

          IImsFactory.createConfigServiceCallback hidl_config_cb = new IImsFactory.createConfigServiceCallback() {
                @Override
                public void onValues(int status, vendor.qti.ims.configservice.V1_0.IConfigService iImsConfigService) {
                    Log.d(LOG_TAG + " V1.0 IConfigService status: ",IImsFactory.StatusCode.toString(status));
                    if(status == IImsFactory.StatusCode.OK) {
                        mConfigService[slotId].setHidlConfigService_1_0(iImsConfigService);
                    }
                    else{
                        // handle scenario when iconfigservice creation fails
                    }
                }
            };

          try {
               mImsFactoryInstance_2_0.createConfigService(slotId+1, mConfigService[slotId].gethidlConfigListener_1_0(), hidl_config_cb);
            } catch (RemoteException | NoSuchElementException e) {
                Log.e(LOG_TAG, "Unable to get to IConfigService_2_0 instance");
            }
        }
        return mConfigService[slotId];
    }

    public SipTransportServiceWrapper getSipTransportService(int slotId) {
        return mSipTransportService[slotId];
    }

    public SipTransportServiceWrapper getSipTransportService(
        int slotId,
        SipTransportServiceWrapper.ImsSipTransportEventListener listener)
    {
        Log.d(LOG_TAG + " V1.1 getSipTransportService slotId: ",Integer.toString(slotId));
        if(mImsFactoryInitialized){
            if(mSipTransportService[slotId] == null) {
                Log.d(LOG_TAG, " mSipTransportService[slotId] is null, so initialize");
                mSipTransportService[slotId] = new
                SipTransportServiceWrapper(slotId, listener);
                initializeRcsSipTransportService(mSipTransportService[slotId]);
            } else {
                Log.d(LOG_TAG, "mSipTransportService[slotId] is Not Null, updating listner");
                mSipTransportService[slotId].setSipTransportEventListener(listener);
            }
        }
        return mSipTransportService[slotId];
    }

    synchronized public void initializeRcsSipTransportService(SipTransportServiceWrapper p) {
     if(p != null)
     {
         if(!getIsSipTransportInited(p.getSlotId()))
         {
            Log.d(LOG_TAG + "SipTransportService initializeSipTransportService slotId: ",
                                      Integer.toString(p.getSlotId()));
            if(mImsFactoryInstance != null)
            {
                Log.d(LOG_TAG, "SipTransportService reinitialize - mImsFactoryInstance is not null");
                getHidlRcsSipTransport_1_2(p);
            }
            else if(mImsFactoryInstance_2_1 != null)
            {
                Log.d(LOG_TAG, "SipTransportService reinitialize - mImsFactoryInstance_2_1 is not null");
                getHidlRcsSipTransport_1_1(p);
            }
            else if(mImsFactoryInstance_2_0 != null)
            {
                Log.d(LOG_TAG, "createRcsSipTransportService: call for mImsFactoryInstance_2_0");
                getHidlRcsSipTransport_1_0(p);
            }
        }
      }
    }

    private void initializeImsPresenceRetry(int slotId) {
        int retryValue = getPresenceSvcRetryCount(slotId);
        if(retryValue < MAX_RETRY_COUNT) {
            retryValue++;
            setPresenceSvcRetryCount(slotId, retryValue);
            Log.d(LOG_TAG + " V1.1 IPresenceService retry Attempt: ", Integer.toString(retryValue));
            //sleep for 500ms
            try {
                Thread.sleep(500);
            }catch(InterruptedException e){
                e.printStackTrace();
            }
            initializeImsPresenceService(slotId);
        } else {
            Log.d(LOG_TAG, "createPresenceService failed: V1.1 IPresenceService MAX_RETRY_COUNT reached");
        }

    }

    synchronized public void initializeImsPresenceService(int slotId)
    {
        if (mPresenceService[slotId] == null) {
            // Presence wrapper only created when createCapabilityExchangeImpl
            // calls getImsPresenceService. If not created, do nothing.
            return;
        }
        Log.d(LOG_TAG, "initializeImsPresenceService start");
        if (mImsFactoryInstance != null) {
            getHidlPresenceService_1_2(slotId);
        } else if (mImsFactoryInstance_2_1 != null) {
            Log.d(LOG_TAG, "initializeImsPresenceService ImsFactory 2_2 not available, falling back to 2_1 ");
            getHidlPresenceService_1_1(slotId);
        } else if (mImsFactoryInstance_2_0 != null) {
            Log.d(LOG_TAG, "initializeImsPresenceService ImsFactory 2_1 not available, falling back to 2_0 ");
            getHidlPresenceService_1_0(slotId);
        } else {
            Log.e(LOG_TAG, "initializeImsPresenceService no factory");
        }
    }

    public PresenceServiceWrapper getImsPresenceService(int slotId) {
        if (!mImsFactoryInitialized) {
            return null;
        }
        if (mPresenceService[slotId] == null) {
            //Create the hidl service object only if needed
            mPresenceService[slotId] = new PresenceServiceWrapper(slotId);
            initializeImsPresenceService(slotId);
        }
        return mPresenceService[slotId];
    }

    private void cleanup() {
        //Note: this will occur on HwBinderThread
        Log.d(LOG_TAG, "Native serviceDied cleanup");
        mImsFactoryInstance = null;
        mImsFactoryInstance_2_1 = null;
        mImsFactoryInstance_2_0 = null;
        mImsFactoryInitialized = false;
        REINTIALIZE_FLAG = 0;
        for(ImsConfigServiceWrapper icsWrapper : mConfigService) {
            try {
                icsWrapper.clear();
                //icsWrapper = null;
                Log.d(LOG_TAG, " ImsConfigServiceWrapper cleanup");
            }
            catch(Exception e){
                Log.d(LOG_TAG, " ImsConfigServiceWrapper cleanup " + e.toString());
            }
        }

        for(PresenceServiceWrapper p : mPresenceService) {
            if(p == null) {
                //Presence Object is null for unsupported sub.
                continue;
            }
            try {
                p.presenceDied();
                Log.d(LOG_TAG, " presenceDied");
            }
            catch(Exception e){
                Log.d(LOG_TAG, "presenceDied caught Excpetion:");
                e.printStackTrace();
             }
        }



        for(OptionsServiceWrapper o : mOptionsService) {
            if(o == null) {
                //Options Object is null for unsupported sub.
                continue;
            }
            try {
                o.optionsDied();
                Log.d(LOG_TAG, " optionsDied");
            }
            catch(Exception e){
              Log.d(LOG_TAG, "optionsDied caught Exception:");
              e.printStackTrace();
            }
        }



        for(SipTransportServiceWrapper p : mSipTransportService) {
            if(p == null) {
                //SipTransportService Object is null for unsupported sub.
                Log.d(LOG_TAG, " SipTransportService obj is null");
                continue;
            }
            try {
                p.sipTransportDied();
                Log.d(LOG_TAG, " sipTransportDied");
            }
            catch(Exception e){
                Log.d(LOG_TAG, "sipTransportDied " + e.toString());
            }
        }
        for(int i=0; i < ImsRcsService.MAX_SLOTS; i++) {
            Log.d(LOG_TAG, " clearing reinit count");
            mIsSipTransportInited[i] = false;
            mSipTransportSvcRetryCount[i] = 0;
            mPresenceSvcRetryCount[i] = 0;
            mPresenceSvcReinitFlag[i] = 0;
        }

        reInitHalServices();
    }

    public void reInitHalServices()
    {
        Log.d(LOG_TAG, "calling reInitHalServices");
        if (REINTIALIZE_FLAG == 0) {
            Log.d(LOG_TAG, "REINTIALIZE_FLAG is 0");
            Initialize();
            for (ImsConfigServiceImpl configImpl : ImsRcsService.getImsConfigService()) {
                Log.d(LOG_TAG, "configImpl call initwrapper");
                configImpl.initConfigWrapper();
            }
            Log.d(LOG_TAG, "REINTIALIZE_FLAG set to 1");
            REINTIALIZE_FLAG = 1;
        }

        for(SipTransportServiceWrapper p : mSipTransportService) {
            if(p == null)
                continue;
            Log.d(LOG_TAG + "SipTransportService reinitialize slotId: ",
                    Integer.toString(p.getSlotId()));
            initializeRcsSipTransportService(p);
        }

        for (int slotId = 0; slotId < ImsRcsService.MAX_SLOTS; slotId++) {
            initializeImsPresenceService(slotId);
            initializeImsOptionsService(slotId);
        }
    }


    public void reinitializeSipTransportService() {
        for(SipTransportServiceWrapper p : mSipTransportService) {
            if(p != null){
                if(!getIsSipTransportInited(p.getSlotId())){
                initializeRcsSipTransportService(p);
            }
        }
    }
    }

    public void reinitializePresenceService() {
        for (int slotId = 0; slotId < ImsRcsService.MAX_SLOTS; slotId++) {
            if(getPresenceSvcReintFlag(slotId) == 0)
            initializeImsPresenceService(slotId);
        }
    }

    public void reinitializeOptionsService() {
        for (int slotId = 0; slotId < ImsRcsService.MAX_SLOTS; slotId++) {
            if(mOptionsService[slotId] != null) {
                initializeImsOptionsService(slotId);
            }
        }
    }

    public void destroyPresenceSevice(int slotId) {
        //@TODO need to check how we can clean-up
        mPresenceService[slotId].close();
        mPresenceService[slotId] = null;
    }

    public void destroyOptionsService(int slotId) {
        //@TODO need to check how we can clean-up
        mOptionsService[slotId].close();
        mOptionsService[slotId] = null;
    }

    public void initializeImsOptionsService(int slotId) {
        Log.d(LOG_TAG, "initializeImsOptionsService start");
        if (mOptionsService[slotId] == null) {
            // Options wrapper only created when createCapabilityExchangeImpl
            // calls getImsOptionsService. If not created, do nothing.
            return;
        }
        if (mImsFactoryInitialized) {
            IImsFactory.createOptionsServiceCallback hidl_options_cb = new IImsFactory.createOptionsServiceCallback() {
                @Override
                public void onValues(int i, IOptionsService iOptionsService) {
                    mOptionsService[slotId].setHidlOptionsService(iOptionsService);
                }
            };
            try {
                if(mImsFactoryInstance != null) {
                    Log.d(LOG_TAG, "initializeImsOptionsService createOptionsService 1.2");
                    mImsFactoryInstance.createOptionsService(slotId+1, mOptionsService[slotId].getHidlOptionsListener(), hidl_options_cb);
                } else if(mImsFactoryInstance_2_1 != null) {
                    Log.d(LOG_TAG, "initializeImsOptionsService createOptionsService 1.1");
                    mImsFactoryInstance_2_1.createOptionsService(slotId+1, mOptionsService[slotId].getHidlOptionsListener(), hidl_options_cb);
                } else if(mImsFactoryInstance_2_0 != null) {
                    Log.d(LOG_TAG, "initializeImsOptionsService createOptionsService 1.0");
                    mImsFactoryInstance_2_0.createOptionsService(slotId+1, mOptionsService[slotId].getHidlOptionsListener(), hidl_options_cb);
                } else {
                    Log.e(LOG_TAG, "initalizeImsOptionsService no factory");
                }
            } catch (RemoteException | NoSuchElementException e) {
                Log.e(LOG_TAG, "Unable to get to IOptionsService instance");
            }
        }
    }

    public OptionsServiceWrapper getImsOptionsService(int slotId) {
        if (mImsFactoryInstance == null && mImsFactoryInstance_2_1 == null && mImsFactoryInstance_2_0 == null) {
            return null;
        }
        if (mOptionsService[slotId] == null) {
            //Create the hidl service object only if needed
            mOptionsService[slotId] = new OptionsServiceWrapper(slotId);
            initializeImsOptionsService(slotId);
        }
        return mOptionsService[slotId];
    }

    synchronized private int getSipTransportSvcRetryCount(int slotId) {
        return mSipTransportSvcRetryCount[slotId];
    }

    synchronized private void setSipTransportSvcRetryCount(int slotId, int value) {
        mSipTransportSvcRetryCount[slotId] = value;
    }

    synchronized private boolean getIsSipTransportInited(int slotId) {
        return mIsSipTransportInited[slotId];
    }

    synchronized private void setSipTransportInited(int slotId, boolean value) {
        mIsSipTransportInited[slotId] = value;
    }

    synchronized private int getPresenceSvcReintFlag(int slotId) {
        return mPresenceSvcReinitFlag[slotId];
    }

    synchronized private void setPresenceSvcReintFlag(int slotId, int value) {
        mPresenceSvcReinitFlag[slotId] = value;
    }

    synchronized private int getPresenceSvcRetryCount(int slotId) {
        return mPresenceSvcRetryCount[slotId];
    }

    synchronized private void setPresenceSvcRetryCount(int slotId, int value) {
        mPresenceSvcRetryCount[slotId] = value;
    }

    private class FactoryDeathRecipient implements IHwBinder.DeathRecipient {
        @Override
        public void serviceDied(long l) {
            ImsRcsServiceMgr.getInstance().cleanup();
        }
    }

    private boolean fallbackToFactory2_1() {
              try{
              mImsFactoryInstance_2_1 = vendor.qti.ims.factory.V2_1.IImsFactory.getService("default", true);
               Log.d(LOG_TAG, "IImsFactory.getService for v2_1 fallback");
              }
              catch (RemoteException | NoSuchElementException e) {
                Log.e(LOG_TAG, "Unable to bind to IImsFactory v2_1 instance");
              }
              if(mImsFactoryInstance_2_1 == null) {
                return false;
              }
              return true;
    }

     private boolean fallbackToFactory2_0() {
             try{
              mImsFactoryInstance_2_0 = vendor.qti.ims.factory.V2_0.IImsFactory.getService("default", true);
               Log.d(LOG_TAG, "IImsFactory.getService for v2_0 fallback");
              }
              catch (RemoteException | NoSuchElementException e) {
                Log.e(LOG_TAG, "Unable to bind to IImsFactory v2_0 instance");
              }
              if(mImsFactoryInstance_2_0 == null)
                return false;

              return true;
     }

     private void getHidlRcsSipTransport_1_0(SipTransportServiceWrapper p) {
         IImsFactory.createRcsSipTransportServiceCallback hidl_sipTransport_cb =
                        new IImsFactory.createRcsSipTransportServiceCallback() {
                            @Override
                            public void onValues(int status,
                                           vendor.qti.ims.rcssip.V1_0.ISipTransportService iSipTransportService)
                            {
                                Log.d(LOG_TAG + " V1.0 ISipTransportService status: ",
                                                    IImsFactory.StatusCode.toString(status));

                                if(status == IImsFactory.StatusCode.OK) {
                                     setSipTransportSvcRetryCount(p.getSlotId(), 0);
                                     setSipTransportInited(p.getSlotId(), true);
                                     p.setHidlSipTransportService(iSipTransportService);
                                } else {
                                     Log.d(LOG_TAG, " V1.0 createRcsSipTransportService failed");
                                }
                            }
        };

        try {
            if(mImsFactoryInstance != null) {
                mImsFactoryInstance.createRcsSipTransportService( p.getSlotId()+1,
                                                     p.getHidlSipTransportListener(),
                                                     hidl_sipTransport_cb);
            }
            else if(mImsFactoryInstance_2_1 != null) {
                mImsFactoryInstance_2_1.createRcsSipTransportService( p.getSlotId()+1,
                                                     p.getHidlSipTransportListener(),
                                                     hidl_sipTransport_cb);
            }
            else if(mImsFactoryInstance_2_0 != null) {
                mImsFactoryInstance_2_0.createRcsSipTransportService(p.getSlotId()+1,
                                                     p.getHidlSipTransportListener(),
                                                     hidl_sipTransport_cb);
            }
            else {
                Log.e(LOG_TAG, "no factory initialized");
            }
        } catch (RemoteException | NoSuchElementException e) {
              Log.e(LOG_TAG, "Unable to get to ISipTransportService::V1_0 instance");
        }
     }

     private void getHidlRcsSipTransport_1_1(SipTransportServiceWrapper p) {
        IImsFactory.createRcsSipTransportService_1_1Callback hidl_sipTransport_1_1_cb =
                     new IImsFactory.createRcsSipTransportService_1_1Callback() {
                         @Override
                         public void onValues(int status,
                              vendor.qti.ims.rcssip.V1_1.ISipTransportService iSipTransportService)
                         {
                              Log.d(LOG_TAG + " V1.1 ISipTransportService status: ",
                                                    IImsFactory.StatusCode.toString(status));
                              if(status == IImsFactory.StatusCode.OK) {
                                  setSipTransportSvcRetryCount(p.getSlotId(), 0);
                                  setSipTransportInited(p.getSlotId(), true);
                                  p.setHidlSipTransportService(iSipTransportService);
                              }
                              else if(status == IImsFactory.StatusCode.FAIL) {
                                  Log.d(LOG_TAG, " V1.1 ISipTransportService status: FAIL ");
                                  int retryCount = getSipTransportSvcRetryCount(p.getSlotId());
                                  if(retryCount < MAX_RETRY_COUNT) {
                                      retryCount++;
                                      setSipTransportSvcRetryCount(p.getSlotId(),retryCount);
                                      Log.d(LOG_TAG + " V1.1 ISipTransportService retry Attempt: ",
                                              Integer.toString(getSipTransportSvcRetryCount(p.getSlotId())));
                                      //sleep for 500ms
                                      try {
                                          Thread.sleep(500);
                                      }catch(InterruptedException e){
                                          e.printStackTrace();
                                      }
                                      initializeRcsSipTransportService(p);
                                  }
                                  else if(retryCount >= MAX_RETRY_COUNT) {
                                      Log.d(LOG_TAG, " V1.1 ISipTransportService MAX_RETRY_COUNT reached");
                                      Log.d(LOG_TAG, " createRcsSipTransportService failed");
                                  }
                              }
                              else if(status == IImsFactory.StatusCode.NOT_SUPPORTED) {
                                  getHidlRcsSipTransport_1_0(p);
                              }
                              else {
                                  Log.d(LOG_TAG, " V1.1 createRcsSipTransportService failed");
                              }
                         }
        };
        try {
            if(mImsFactoryInstance != null) {
                mImsFactoryInstance.createRcsSipTransportService_1_1( p.getSlotId()+1,
                                                     p.getHidlSipTransportListener(),
                                                     hidl_sipTransport_1_1_cb);
            }
            else if(mImsFactoryInstance_2_1 != null) {
                mImsFactoryInstance_2_1.createRcsSipTransportService_1_1( p.getSlotId()+1,
                                                     p.getHidlSipTransportListener(),
                                                     hidl_sipTransport_1_1_cb);
            }
            else {
                Log.e(LOG_TAG, "no factory initialized 1.1 createRcsSip");
            }
        } catch (RemoteException | NoSuchElementException e) {
              Log.e(LOG_TAG, "Unable to get to ISipTransportService::V1_1 instance");
        }
     }

     private void getHidlRcsSipTransport_1_2(SipTransportServiceWrapper p) {
        IImsFactory.createRcsSipTransportService_1_2Callback hidl_sipTransport_1_2_cb =
            new IImsFactory.createRcsSipTransportService_1_2Callback() {
                @Override
                public void onValues(int status,ISipTransportService iSipTransportService)
                {
                    Log.d(LOG_TAG + " V1.2 ISipTransportService status: ",IImsFactory.StatusCode.toString(status));
                    if(status == IImsFactory.StatusCode.OK) {
                        setSipTransportSvcRetryCount(p.getSlotId(), 0);
                        setSipTransportInited(p.getSlotId(), true);
                        p.setHidlSipTransportService(iSipTransportService);
                    }
                    else if(status == IImsFactory.StatusCode.FAIL) {
                        Log.d(LOG_TAG, " V1.2 ISipTransportService status: FAIL ");
                        int retryCount = getSipTransportSvcRetryCount(p.getSlotId());
                        if(retryCount < MAX_RETRY_COUNT) {
                            retryCount++;
                            setSipTransportSvcRetryCount(p.getSlotId(),retryCount);
                            Log.d(LOG_TAG + " V1.2 ISipTransportService retry Attempt: ",
                              Integer.toString(getSipTransportSvcRetryCount(p.getSlotId())));
                            //sleep for 500ms
                            try {
                                Thread.sleep(500);
                            }catch(InterruptedException e){
                                e.printStackTrace();
                            }
                            initializeRcsSipTransportService(p);
                        }
                        else if(retryCount >= MAX_RETRY_COUNT) {
                            Log.d(LOG_TAG, " V1.2 ISipTransportService MAX_RETRY_COUNT reached");
                            Log.d(LOG_TAG, " createRcsSipTransportService failed");
                        }
                    }
                    else if(status == IImsFactory.StatusCode.NOT_SUPPORTED) {
                        getHidlRcsSipTransport_1_1(p);
                    }
                    else {
                        Log.d(LOG_TAG, " V1.2 createRcsSipTransportService failed");
                    }
                }
        };
        try {
            if(mImsFactoryInstance != null) {
                mImsFactoryInstance.createRcsSipTransportService_1_2( p.getSlotId()+1,
                                                     p.getHidlSipTransportListener(),
                                                     hidl_sipTransport_1_2_cb);
            }
            else {
                Log.e(LOG_TAG, "no factory initialized for 1.2 createRcsSip");
            }
        } catch (RemoteException | NoSuchElementException e) {
              Log.e(LOG_TAG, "Unable to get to ISipTransportService::V1_2 instance");
        }
     }

     private void getHidlPresenceService_1_0(int slotId) {
         Log.i(LOG_TAG, "getHidlPresenceService_1_0 enter");
         IImsFactory.createPresenceServiceCallback hidl_presence_cb = new IImsFactory.createPresenceServiceCallback() {
            @Override
            public void onValues(
                    int status,
                    vendor.qti.ims.rcsuce.V1_0.IPresenceService iPresenceService) {
                Log.d(LOG_TAG + " V1.0 IPresenceService status: ", IImsFactory.StatusCode.toString(status));
                if (status == IImsFactory.StatusCode.OK) {
                    setPresenceSvcRetryCount(slotId, 0);
                    setPresenceSvcReintFlag(slotId, 1);
                    mPresenceService[slotId].setHidlPresenceService(iPresenceService);
                } else {
                    Log.d(LOG_TAG, " V1.0 createPresenceService failed");
                }
            }
        };

        try {
            if(mImsFactoryInstance != null) {
                mImsFactoryInstance.createPresenceService(slotId+1,
                   mPresenceService[slotId].getHidlPresenceListener(), hidl_presence_cb);
            } else if(mImsFactoryInstance_2_1 != null) {
                mImsFactoryInstance_2_1.createPresenceService(slotId+1,
                   mPresenceService[slotId].getHidlPresenceListener(), hidl_presence_cb);
            } else if(mImsFactoryInstance_2_0 != null) {
                mImsFactoryInstance_2_0.createPresenceService(slotId+1,
                   mPresenceService[slotId].getHidlPresenceListener(), hidl_presence_cb);
            } else {
                Log.e(LOG_TAG, "no factory initialized");
            }
        } catch (RemoteException | NoSuchElementException ex) {
            Log.e(LOG_TAG, "Unable to get to IPresenceService::V1_0 instance");
        }
     }

     private void getHidlPresenceService_1_1(int slotId) {
        Log.i(LOG_TAG, "getHidlPresenceService_1_1 enter");
        IImsFactory.createPresenceService_1_1Callback hidl_presence_1_1_cb = new IImsFactory.createPresenceService_1_1Callback() {
            @Override
            public void onValues(
                    int status,
                    vendor.qti.ims.rcsuce.V1_1.IPresenceService iPresenceService) {
                Log.d(LOG_TAG + " V1.1 IPresenceService status: ", IImsFactory.StatusCode.toString(status));
                if (status == IImsFactory.StatusCode.OK) {
                    setPresenceSvcRetryCount(slotId, 0);
                    setPresenceSvcReintFlag(slotId, 1);
                    mPresenceService[slotId].setHidlPresenceService(iPresenceService);
                }
                else if(status == IImsFactory.StatusCode.FAIL) {
                    Log.d(LOG_TAG, " V1.1 IPresenceService status: FAIL ");
                    initializeImsPresenceRetry(slotId);
                }
                else if(status == IImsFactory.StatusCode.NOT_SUPPORTED) {
                    getHidlPresenceService_1_0(slotId);
                }
            }
        };

        try {
            if(mImsFactoryInstance != null) {
                mImsFactoryInstance.createPresenceService_1_1(slotId+1,
                   mPresenceService[slotId].getHidlPresenceListener(), hidl_presence_1_1_cb);
            } else if(mImsFactoryInstance_2_1 != null) {
                mImsFactoryInstance_2_1.createPresenceService_1_1(slotId+1,
                   mPresenceService[slotId].getHidlPresenceListener(), hidl_presence_1_1_cb);
            } else {
                Log.e(LOG_TAG, "no factory initialized for createPresenceService_1_1");
            }
        } catch (RemoteException | NoSuchElementException ex) {
            Log.e(LOG_TAG, "Unable to get to IPresenceService::V1_1 instance");
        }
     }

     private void getHidlPresenceService_1_2(int slotId) {
        Log.i(LOG_TAG, "getHidlPresenceService_1_2 enter");
        IImsFactory.createPresenceService_1_2Callback hidl_presence_1_2_cb = new IImsFactory.createPresenceService_1_2Callback() {
            @Override
            public void onValues(
                    int status,
                    vendor.qti.ims.rcsuce.V1_2.IPresenceService iPresenceService) {
                Log.d(LOG_TAG + " V1.2 IPresenceService status: ", IImsFactory.StatusCode.toString(status));
                if (status == IImsFactory.StatusCode.OK) {
                    setPresenceSvcRetryCount(slotId, 0);
                    setPresenceSvcReintFlag(slotId, 1);
                    mPresenceService[slotId].setHidlPresenceService(iPresenceService);
                }
                else if(status == IImsFactory.StatusCode.FAIL) {
                    Log.d(LOG_TAG, " V1.2 IPresenceService status: FAIL ");
                    initializeImsPresenceRetry(slotId);
                }
                else if(status == IImsFactory.StatusCode.NOT_SUPPORTED) {
                    getHidlPresenceService_1_1(slotId);
                }
            }
        };

        try {
            if(mImsFactoryInstance != null) {
                mImsFactoryInstance.createPresenceService_1_2(slotId+1,
                   mPresenceService[slotId].getHidlPresenceListener(), hidl_presence_1_2_cb);
            } else {
                Log.e(LOG_TAG, "no factory initialized for createPresenceService_1_2");
            }
        } catch (RemoteException | NoSuchElementException ex) {
            Log.e(LOG_TAG, "Unable to get to IPresenceService::V1_2 instance");
        }
     }

     private boolean isInterfaceListedInManifest(String interfaceName) {
         boolean found = false;
         Log.d(LOG_TAG," isInterfaceListedInManifest is started with string : "+interfaceName);
         try {
         IServiceManager sm = IServiceManager.getService();

         List<String> services = sm.listManifestByInterface(interfaceName);
         for(String str: services)
             Log.e(LOG_TAG, " service running for imsfactory: "+str);
         return !services.isEmpty();
         } catch(RemoteException e) {
             Log.e(LOG_TAG, " error while trying to find factory version in vintf");
         }
         return false;

     }
}
