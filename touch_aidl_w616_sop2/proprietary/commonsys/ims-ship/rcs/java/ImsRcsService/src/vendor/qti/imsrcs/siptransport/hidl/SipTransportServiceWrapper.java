/**********************************************************************
 * Copyright (c) 2021-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsrcs.siptransport.hidl;

import android.os.RemoteException;


import java.util.ArrayList;
import java.util.Date;
import java.util.concurrent.Executor;
import java.util.concurrent.ThreadLocalRandom;
import android.util.Log;

import vendor.qti.ims.configservice.V1_0.IConfigService;
import vendor.qti.ims.factory.V2_2.IImsFactory;
import vendor.qti.ims.rcssip.V1_2.ISipConnection;
import vendor.qti.ims.rcssip.V1_0.ISipTransportListener;
import vendor.qti.ims.rcssip.V1_2.ISipTransportService;
import vendor.qti.ims.rcssip.V1_0.configData;
import vendor.qti.ims.rcssip.V1_0.keyValuePairStringType;
import vendor.qti.ims.rcssip.V1_0.regRestorationDataKeys;


public class SipTransportServiceWrapper {

    private int userData = 1000;
    public vendor.qti.ims.rcssip.V1_0.ISipTransportService mHidlSipTransportService = null;
    public vendor.qti.ims.rcssip.V1_1.ISipTransportService mHidlSipTransportService_1 = null;
    public ISipTransportService mHidlSipTransportService_1_2 = null;
    public ImsSipTransportEventListener mSipTransportEventListener;
    public int mServiceStatus = -1;
    SipTransportListener mhidlSipTransportListener = new
        SipTransportListener();
    final String LOG_TAG = "SipTransportServiceWrapper";
    private static int INVALID_SLOT_ID = -1;

    private int mSlotId = INVALID_SLOT_ID;

    private int mHalVersion = 2; //V1_2

    public SipTransportServiceWrapper(int slotId,
                       ImsSipTransportEventListener listener) {
        Log.d(LOG_TAG, ": ctor for slotId["+Integer.toString(slotId)+"]");
        mSlotId = slotId;
        mSipTransportEventListener = listener;
    }

    public void setHidlSipTransportService(vendor.qti.ims.rcssip.V1_0.ISipTransportService service) {
        Log.d(LOG_TAG, ": setHidlSipTransportService called");
        if(service !=null)
            Log.d(LOG_TAG, ": setHidlSipTransportService"+
                           " service obj not null");
        mHidlSipTransportService = service;
    }

    public void setHidlSipTransportService(vendor.qti.ims.rcssip.V1_1.ISipTransportService service) {
        Log.d(LOG_TAG, ": setHidlSipTransportService clled");
        if(service !=null)
            Log.d(LOG_TAG, ": setHidlSipTransportService"+
                           " service obj not null");
        mHidlSipTransportService_1 = service;
    }

    public void setHidlSipTransportService(ISipTransportService service) {
        Log.d(LOG_TAG, ": setHidlSipTransportService clled");
        if(service !=null)
            Log.d(LOG_TAG, ": setHidlSipTransportService"+
                           " service obj not null");
        mHidlSipTransportService_1_2 = service;
    }

    public void setHalVersion(int version) {
        mHalVersion = version;
    }

    public void setSipTransportEventListener(
        ImsSipTransportEventListener listener) {
        Log.d(LOG_TAG, ": setSipTransportEventListener, mServiceStatus:"+mServiceStatus);
        mSipTransportEventListener = listener;
        if(mServiceStatus == 0)
        {
            Log.d(LOG_TAG, ": calling handleServiceStatus");
            mSipTransportEventListener.handleServiceStatus(mServiceStatus);
        }
    }

    public ISipTransportListener getHidlSipTransportListener() {
        return mhidlSipTransportListener;
    }

    public int getSlotId() {
        return mSlotId;
    }

    public int getHalVersion() {
        return mHalVersion;
    }

    public void sipTransportDied() {
        Log.d(LOG_TAG, ": sipTransportDied");
        if(mSipTransportEventListener != null)
           mSipTransportEventListener.handleSipTransportServiceDied();
        if (mHidlSipTransportService != null)
        mHidlSipTransportService = null;
        else if(mHidlSipTransportService_1 != null)
           mHidlSipTransportService_1 = null;
        else if(mHidlSipTransportService_1_2 != null)
            mHidlSipTransportService_1_2 = null;
    }

    public int createConnection(
        String featureTagsString,
        SipDelegateWrapper mSipDelegateWrapper) {
        userData++;
        Log.d(LOG_TAG, ": createConnection - userData:"+userData);
        if(mHidlSipTransportService_1_2 != null) {
            ISipTransportService.createConnection_1_2Callback hidl_createConnection_1_2_cb =
                new ISipTransportService.createConnection_1_2Callback() {
                    @Override
                    public void onValues(
                        int i,
                        ISipConnection iSipConnection,
                        long l) {
                        Log.d(LOG_TAG, ": createConnection_1_2Callback onValues");
                        mSipDelegateWrapper.setHidlSipConnection(
                            iSipConnection);
                        mSipDelegateWrapper.setHidlConnectionHandle(l);
                    }
            };

            try {
                Log.d(LOG_TAG, ": createConnection_1_1 - Before calling hidl fns:");
                mHidlSipTransportService_1_2.createConnection_1_2(
                    mSipDelegateWrapper.getHidlSipConnectionListener(),
                    featureTagsString,
                    userData,
                    hidl_createConnection_1_2_cb);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, ": Exception- Unable to createConnection_1_1 : "+e);
                e.printStackTrace();
                return -1;
            }
        }
        else if(mHidlSipTransportService_1 != null) {
            ISipTransportService.createConnection_1_1Callback hidl_createConnection_1_1_cb =
                new ISipTransportService.createConnection_1_1Callback() {
                    @Override
                    public void onValues(
                        int i,
                        vendor.qti.ims.rcssip.V1_1.ISipConnection iSipConnection,
                        long l) {
                        Log.d(LOG_TAG, ": createConnection_1_1Callback onValues");
                        mSipDelegateWrapper.setHidlSipConnection(
                            iSipConnection);
                        mSipDelegateWrapper.setHidlConnectionHandle(l);
                    }
            };

            try {
                Log.d(LOG_TAG, ": createConnection_1_1 - Before calling hidl fns:");
                mHidlSipTransportService_1.createConnection_1_1(
                    mSipDelegateWrapper.getHidlSipConnectionListener(),
                    featureTagsString,
                    userData,
                    hidl_createConnection_1_1_cb);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, ": Exception- Unable to createConnection_1_1 : "+e);
                e.printStackTrace();
                return -1;
            }
        }
        else if(mHidlSipTransportService != null) {
            vendor.qti.ims.rcssip.V1_0.ISipTransportService.createConnectionCallback hidl_createConnection_cb =
            new vendor.qti.ims.rcssip.V1_0.ISipTransportService.createConnectionCallback() {
                    @Override
                    public void onValues(
                        int i,
                    vendor.qti.ims.rcssip.V1_0.ISipConnection iSipConnection,
                        long l) {
                        Log.d(LOG_TAG, ": createConnectionCallback onValues");
                        mSipDelegateWrapper.setHidlSipConnection(
                            iSipConnection);
                        mSipDelegateWrapper.setHidlConnectionHandle(l);
                    }
            };

            try {
                Log.d(LOG_TAG, ": createConnection - Before calling hidl fns:");
                mHidlSipTransportService.createConnection(
                    mSipDelegateWrapper.getHidlSipConnectionListener(),
                    featureTagsString,
                    userData,
                    hidl_createConnection_cb);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, ": Exception- Unable to createConnection : "+e);
                e.printStackTrace();
                return -1;
            }
        } else {
            Log.e(LOG_TAG, ": createConnection - HidlSipTransportService object not initialized");
            return -1;
        }

        return userData;
    }

    public int closeConnection(
        int reason, SipDelegateWrapper mSipDelegateWrapper) {
        Log.d(LOG_TAG, ": closeConnection");
        userData++;
        if(mHidlSipTransportService_1_2 != null) {
            try {
                mHidlSipTransportService_1_2.closeConnection(
                    mSipDelegateWrapper.getHidlConnectionHandle(),
                    reason,
                    userData);
            } catch (RemoteException e) {
                e.printStackTrace();
                return -1;
            }
        } else if(mHidlSipTransportService_1 != null) {
            try {
                mHidlSipTransportService_1.closeConnection(
                    mSipDelegateWrapper.getHidlConnectionHandle(),
                    reason,
                    userData);
            } catch (RemoteException e) {
                e.printStackTrace();
                return -1;
            }
        } else if(mHidlSipTransportService != null) {
            try {
                mHidlSipTransportService.closeConnection(
                    mSipDelegateWrapper.getHidlConnectionHandle(),
                    reason,
                    userData);
            } catch (RemoteException e) {
                e.printStackTrace();
                return -1;
            }
        } else {
            Log.e(LOG_TAG, ": closeConnection - HidlSipTransportService object not initialized");
            return -1;
        }
        return userData;
    }

    public int triggerRegistration() {
        Log.d(LOG_TAG, ": triggerRegistration");
        userData++;
        if(mHidlSipTransportService_1_2 != null) {
            try {
                mHidlSipTransportService_1_2.triggerRegistration(userData);
            } catch (RemoteException e) {
                e.printStackTrace();
                return -1;
            }
        }
        else if(mHidlSipTransportService_1 != null) {
            try {
                mHidlSipTransportService_1.triggerRegistration(userData);
            } catch (RemoteException e) {
                e.printStackTrace();
                return -1;
            }
        }
        else if(mHidlSipTransportService != null) {
            try {
                mHidlSipTransportService.triggerRegistration(userData);
            } catch (RemoteException e) {
                e.printStackTrace();
                return -1;
            }
        } else {
            Log.e(LOG_TAG, ": triggerReg - HidlSipTransportService object not initialized");
            return -1;
        }
        return userData;
    }

    public int triggerRegRestoration(int sipCode, String sipReason) {
        Log.d(LOG_TAG, ": triggerRegRestoration");
        userData++;
        ArrayList<keyValuePairStringType> regRestorationData = new
            ArrayList<>();
        keyValuePairStringType regData = new keyValuePairStringType();
        regData.key = regRestorationDataKeys.responseCode;
        regData.value = Integer.toString(sipCode);
        regRestorationData.add(regData);
        if(mHidlSipTransportService_1_2 != null) {
            try {
                mHidlSipTransportService_1_2.triggerRegRestoration(
                    regRestorationData,userData);
            } catch (RemoteException e) {
                e.printStackTrace();
                return -1;
            }
        } else if(mHidlSipTransportService_1 != null) {
            try {
                mHidlSipTransportService_1.triggerRegRestoration(
                    regRestorationData,userData);
            } catch (RemoteException e) {
                e.printStackTrace();
                return -1;
            }
        } else if(mHidlSipTransportService != null) {
            try {
                mHidlSipTransportService.triggerRegRestoration(
                    regRestorationData,userData);
            } catch (RemoteException e) {
                e.printStackTrace();
                return -1;
            }
        } else {
            Log.e(LOG_TAG, ": triggerRegRestoration - HidlSipTransportService object not initialized");
            return -1;
        }
        return userData;
    }

    public int clearSipTransportService() {
        Log.d(LOG_TAG, ": clearSipTransportService");
        if(mHidlSipTransportService_1_2 != null) {
            try {
                return mHidlSipTransportService_1_2.clearSipTransportService();
            } catch (RemoteException e) {
                e.printStackTrace();
                return -1;
            }
        } else if(mHidlSipTransportService_1 != null) {
            try {
                return mHidlSipTransportService_1.clearSipTransportService();
            } catch (RemoteException e) {
                e.printStackTrace();
                return -1;
            }
        } else if(mHidlSipTransportService != null) {
            try {
                return mHidlSipTransportService.clearSipTransportService();
            } catch (RemoteException e) {
                e.printStackTrace();
                return -1;
            }
        } else {
            Log.e(LOG_TAG, ": clearSipTransportService - HidlSipTransportService object not initialized");
            return -1;
        }
    }

    private class SipTransportListener extends ISipTransportListener.Stub {

        @Override
        public void onServiceStatus(int i) throws RemoteException {
            mServiceStatus = i;
            if(mSipTransportEventListener != null)
                mSipTransportEventListener.handleServiceStatus(i);
        }

        @Override
        public void onConfigurationChange(
            configData configData) throws RemoteException {
            if(mSipTransportEventListener != null)
                mSipTransportEventListener.handleConfigurationChanged(
                    configData);
        }

        @Override
        public void onCommandStatus(
            int cmdStatus, int userData) throws RemoteException {
            Log.d(LOG_TAG, "onCommandStatus received with userdata: "+userData);
            if(mSipTransportEventListener != null)
                mSipTransportEventListener.handleCmdStatus(
                    cmdStatus, userData);
        }
    };


    public static abstract class ImsSipTransportEventListener {
        protected Executor mExecutor;
        static final String LOG_TAG = "ImsSipTransportEventListener";
        public ImsSipTransportEventListener(Executor e) {
            mExecutor = e;
            Log.d(LOG_TAG, ": ImsSipTransportEventListener");
        }

        public final void handleServiceStatus(int s) {
            mExecutor.execute(()->{
                onServiceStatus(s);
            });
        }

        public final void handleSipTransportServiceDied() {
            mExecutor.execute(()->{
                onSipTransportServiceDied();
            });
        }

        public final void handleCmdStatus(int status,int userdata) {
            mExecutor.execute(()->{
                onCmdStatus(status,userdata);
            });
        }

        public final void handleConfigurationChanged(configData config) {
            mExecutor.execute(()->{
                onConfigurationChange(config);
            });
        }
        public void onServiceStatus(int status) {}
        public void onSipTransportServiceDied(){}
        public void onCmdStatus(int status,int userdata) {}
        public void onConfigurationChange(configData configData) {}
    };

}
