/**********************************************************************
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel;

import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import vendor.qti.ims.datachannelservice.IDataChannelService;
import vendor.qti.imsdatachannel.impls.ImsDataChannelServiceImpl;
import vendor.qti.imsdatachannel.wrappers.ImsDataChannelServiceWrapper;
import android.os.IBinder;
import android.os.IBinder.DeathRecipient;
import java.util.*;
import android.os.ServiceManager;
import android.util.Log;
import android.os.RemoteException;

public class ImsDataChannelServiceMgr {
    final String LOG_TAG = ImsDataChannelService.LOG_TAG + "_ImsDataChannelServiceManager";

    private static ImsDataChannelServiceMgr mMgr;
    int MAX_RETRY_COUNT = 10;
    int REINTIALIZE_FLAG = 0;
    int[] mSipTransportSvcRetryCount;
    int[] mPresenceSvcReinitFlag;
    int[] mPresenceSvcRetryCount;
    boolean[] mIsSipTransportInited;
    vendor.qti.ims.datachannelservice.IDataChannelService mAidlDcService = null;

    private ImsDataChannelServiceWrapper mDataChannelServiceWrapper;
    private ImsDataChannelServiceImpl mImsDataChannelServiceImpl = null;

    private Executor mServiceExecutor = null;

    private static final String ImsDataChannelString = "vendor.qti.ims.datachannelservice.IDataChannelService/default";

    private ImsDataChannelServiceMgr() {
        //mDataChannelServiceWrapper = new ImsDataChannelServiceWrapper();
    }

    synchronized public static ImsDataChannelServiceMgr getInstance() {
        if (mMgr == null) {
            mMgr = new ImsDataChannelServiceMgr();
        }
        return mMgr;
    }

    public void dispose() {

    }

    public void initialize(Executor mServiceExecutor) {
        this.mServiceExecutor = mServiceExecutor;
        startQueryForAidlService();
    }

    public void startQueryForAidlService() {
        if (mServiceExecutor == null) {
            Log.e(LOG_TAG,"service executor is null!!");
            return;
        }
        mServiceExecutor.execute(()-> {
            Log.d(LOG_TAG, "initializing ImsDataChannelServiceManager");
            queryForAidlService();
        });
    }

    public boolean queryForAidlService(){
        //Log.d(LOG_TAG, "initializing ImsDataChannelServiceManager");
        try {
            Log.d(LOG_TAG, "calling IDataChannelService getService");
            mAidlDcService = vendor.qti.ims.datachannelservice.IDataChannelService.Stub.asInterface(ServiceManager.waitForService(ImsDataChannelString));
            if (mAidlDcService == null) {
                Log.e(LOG_TAG, "initialize() mAidlDcService got null");
            } else {
                Log.d(LOG_TAG, "initialize() mAidlDcService got service " + mAidlDcService.toString());
            }
        } catch(Exception e) {
            Log.e(LOG_TAG, ": Exception- Unable to get Aidl DataChannelService : "+e);
            e.printStackTrace();
            return false;
        }

        try {
            Log.e(LOG_TAG, "setDeathRecipient for dcservice");
            mAidlDcService.asBinder().linkToDeath(new DcServiceDeathReceipient(),-1);
        } catch (RemoteException e) {
            e.printStackTrace();
        }

        if (mDataChannelServiceWrapper == null) {
            mDataChannelServiceWrapper = new ImsDataChannelServiceWrapper(mAidlDcService);
            Log.e(LOG_TAG, "creating ImsDataChannelServiceWrapper");
        }
        else {
            mDataChannelServiceWrapper.setAidlDcService(mAidlDcService);
            Log.e(LOG_TAG, "ImsDataChannelServiceWrapper exists, updating the Aidl instance!!");
        }
        mImsDataChannelServiceImpl.setServiceWrapperInst(mDataChannelServiceWrapper);

        //this will initiate getavailability request to native service, in case of service restart or SSR.
        mImsDataChannelServiceImpl.getAllServiceAvailability();
        return true;
    }

    public ImsDataChannelServiceWrapper getImsDataChannelService(){
        //mDataChannelServiceWrapper = new ImsDataChannelServiceWrapper();
        //mDataChannelServiceWrapper.setAidlDcService(mAidlDcService);
        return mDataChannelServiceWrapper;
    }

    public ImsDataChannelServiceImpl getmImsDataChannelServiceImpl(){
        return mImsDataChannelServiceImpl;
    }

    public IBinder getImsDataChannelServiceBinder() {
        Log.i(LOG_TAG, "return binder.");
        return mImsDataChannelServiceImpl.getBinder();
    }

    public void createImsDataServiceImpl(int numSlots){
        mImsDataChannelServiceImpl = new ImsDataChannelServiceImpl(numSlots);
    }

    /*
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
    */

    private class DcServiceDeathReceipient implements IBinder.DeathRecipient {
        @Override
        public void binderDied() {
             mAidlDcService.asBinder().unlinkToDeath(this,0);
            ImsDataChannelServiceMgr.getInstance().cleanupAidlService();
        }
    }

    /*it will notify the clients that data channel service is unavailable when native service dies */
    public void cleanupAidlService() {
        mImsDataChannelServiceImpl.cleanupAidlService();
    }

    // Invoked on service onDestroy()
    public void cleanup(){
        mImsDataChannelServiceImpl.cleanUp();
        mImsDataChannelServiceImpl = null;
    }
}
