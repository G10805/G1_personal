/**********************************************************************
 * Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsrcs.siptransport.hidl;

import android.os.RemoteException;
import android.util.Log;

import java.util.ArrayList;
import java.util.concurrent.Executor;

import vendor.qti.ims.rcssip.V1_2.ISipConnection;
import vendor.qti.ims.rcssip.V1_2.ISipConnectionListener;
import vendor.qti.ims.rcssip.V1_2.ISipTransportService;
import vendor.qti.ims.rcssip.V1_0.SipTransportStatusCode;
import vendor.qti.ims.rcssip.V1_0.featureTagData;
import vendor.qti.ims.rcssip.V1_2.eventData;
import vendor.qti.ims.rcssip.V1_0.messageData;
import vendor.qti.ims.rcssip.V1_0.sessionData;
import vendor.qti.ims.rcssip.V1_2.rcsSessionStatus;

public class SipDelegateWrapper {

    public SipDelegateWrapper.ImsSipDelegateEventListener
        mSipDelegateEventListener;
    private ISipConnection mHidlSipConnection = null;
    private vendor.qti.ims.rcssip.V1_1.ISipConnection mHidlSipConnection_1_1 = null;
    private vendor.qti.ims.rcssip.V1_0.ISipConnection mHidlSipConnection_1_0 = null;
    private SipDelegateListener mHidlSipConnectionListener = new
        SipDelegateListener();
    private int mDestroyReason;
    private long mConnectionHandle = 0;
    final String LOG_TAG = "SipDelegateWrapper";

    public SipDelegateWrapper(){

    }

    public ISipConnection getHidlSipConnection() {
        return mHidlSipConnection;
    }

    public vendor.qti.ims.rcssip.V1_1.ISipConnection getHidlSipConnection_1_1() {
        return mHidlSipConnection_1_1;
    }

    public vendor.qti.ims.rcssip.V1_0.ISipConnection getHidlSipConnection_1_0() {
        return mHidlSipConnection_1_0;
    }

    public ISipConnectionListener getHidlSipConnectionListener() {
        Log.d(LOG_TAG, ": getHidlSipConnectionListener invoked");
        return mHidlSipConnectionListener;
    }

    public void setHidlConnectionHandle(long handle) {
        mConnectionHandle = handle;
    }

   public long getHidlConnectionHandle(long handle) {
        return mConnectionHandle;
    }

    public void setSipDelegateEventListener(
        ImsSipDelegateEventListener listener){
        mSipDelegateEventListener = listener;
    }
    public long getHidlConnectionHandle() {
        return mConnectionHandle;
    }

    public void setHidlSipConnection(ISipConnection iSipConnection) {
        mHidlSipConnection = iSipConnection;
    }

    public void setHidlSipConnection(vendor.qti.ims.rcssip.V1_1.ISipConnection iSipConnection) {
        mHidlSipConnection_1_1 = iSipConnection;
    }

    public void setHidlSipConnection(vendor.qti.ims.rcssip.V1_0.ISipConnection iSipConnection) {
        mHidlSipConnection_1_0 = iSipConnection;
    }

    public void handleCloseTransaction(String callId, int userData) {
        if(mHidlSipConnection != null) {
            try{
                mHidlSipConnection.closeTransaction(callId,userData);
            }catch (RemoteException e) {
                e.printStackTrace();
            }
        }else if(mHidlSipConnection_1_1 != null) {
            try{
                mHidlSipConnection_1_1.closeTransaction(callId,userData);
            }catch (RemoteException e) {
                e.printStackTrace();
            }
        }else if(mHidlSipConnection_1_0 != null) {
            try{
                mHidlSipConnection_1_0.closeTransaction(callId,userData);
            }catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    public void handleSendMessage(messageData msg, int userData) {
        if(mHidlSipConnection != null) {
            try {
                 Log.i(LOG_TAG,"sendMessage::before calling hidl sendMsg API");
                 mHidlSipConnection.sendMessage(msg, userData);
            } catch (RemoteException e) {
                 e.printStackTrace();
            }
        } else if(mHidlSipConnection_1_1 != null) {
            try {
                Log.i(LOG_TAG,"sendMessage::before calling hidl sendMsg API");
                mHidlSipConnection_1_1.sendMessage(msg, userData);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else if(mHidlSipConnection_1_0 != null) {
            try {
                Log.i(LOG_TAG,"sendMessage::before calling hidl sendMsg API");
                mHidlSipConnection_1_0.sendMessage(msg, userData);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    public void handleUpdateSession(String callId, ArrayList<sessionData> sessionContent) {
        if(mHidlSipConnection != null) {
            try {
                mHidlSipConnection.updateSessionContent(callId, sessionContent);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else if(mHidlSipConnection_1_1 != null) {
            try {
                mHidlSipConnection_1_1.updateSessionContent(callId, sessionContent);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        } else if(mHidlSipConnection_1_0 != null) {
            try {
                mHidlSipConnection_1_0.updateSessionContent(callId, sessionContent);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    public void handleHoldSessionStatus(String featureTag, int userData) {
        if(mHidlSipConnection != null) {
            try {
                mHidlSipConnection.setStatus(featureTag, rcsSessionStatus.RCS_SERVICE_STATUS_HOLD, userData);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    public void handleActiveSessionStatus(String featureTag, int userData) {
        if(mHidlSipConnection != null) {
            try {
                mHidlSipConnection.setStatus(featureTag, rcsSessionStatus.RCS_SERVICE_STATUS_ACTIVE, userData);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    public void handleInactiveSessionStatus(String featureTag, int userData) {
        if(mHidlSipConnection != null) {
            try {
                mHidlSipConnection.setStatus(featureTag, rcsSessionStatus.RCS_SERVICE_STATUS_INACTIVE, userData);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }
    private class SipDelegateListener extends ISipConnectionListener.Stub {

        @Override
        public void onConnectionCreated(
            int status,
            ArrayList<featureTagData> deniedFTList) throws RemoteException {
            mSipDelegateEventListener.handleDelegateCreation(status, deniedFTList);
        }

        @Override
        public void onConnectionDestroyed(
            int status, int reason) throws RemoteException {
            mSipDelegateEventListener.handleDelegateDestroyed(status, reason);
        }

        @Override
        public void onEventReceived(int i) throws RemoteException {
            mSipDelegateEventListener.handleOnEventReceived(i);
        }

        @Override
        public void onEventReceived_1_1(vendor.qti.ims.rcssip.V1_1.eventData data) throws RemoteException {
            mSipDelegateEventListener.handleOnEventReceived_1_1(data);
        }

        @Override
        public void onEventReceived_1_2(eventData data) throws RemoteException {
            mSipDelegateEventListener.handleOnEventReceived_1_2(data);
        }

        @Override
        public void handleIncomingMessage(
            ArrayList<Byte> arrayList) throws RemoteException {
            Log.d(LOG_TAG, ": handleIncomingMessage invoked");
            mSipDelegateEventListener.handleIncomingMsg(arrayList);
        }

        @Override
        public void onCommandStatus(int status, int userdata)
            throws RemoteException {
                Log.d(LOG_TAG, ": onCommandStatus handleSipDelegateCmdStatus invoked");
            mSipDelegateEventListener.handleSipDelegateCmdStatus(
                status,
                userdata);
        }

        @Override
        public void onFeatureTagStatusChange(
            ArrayList<featureTagData> arrayList) throws RemoteException {
            mSipDelegateEventListener.handleFeatureTagStatusChange(
                    arrayList);
        }
    };

    public static abstract class ImsSipDelegateEventListener {
        protected Executor mExecutor;
        public ImsSipDelegateEventListener(Executor e) {
            mExecutor = e;
        }

        public final void handleSipDelegateCmdStatus(
            int status, int userData) {
            mExecutor.execute(()-> {
                onConnectionCmdStatus(status, userData);
            });
        }

        public final void handleIncomingMsg(ArrayList<Byte> sipMsg) {
            mExecutor.execute(()-> {
                onIncomingMessageReceived(sipMsg);
            });
        }

        public final void handleOnEventReceived(int connectionStatus) {
            mExecutor.execute(()->{
                onEventReceived(connectionStatus);
            });
        }

        public final void handleOnEventReceived_1_1(vendor.qti.ims.rcssip.V1_1.eventData evtData) {
            mExecutor.execute(()->{
                onEventReceived_1_1(evtData);
            });
        }

        public final void handleOnEventReceived_1_2(eventData evtData) {
            mExecutor.execute(()->{
                onEventReceived_1_2(evtData);
            });
        }

        public final void handleFeatureTagStatusChange(
            ArrayList<featureTagData> featureTagList) {
            mExecutor.execute(()->{
                onFeatureTagStatusChange(featureTagList);
            });
        }

        public final void handleDelegateCreation(
            int status, ArrayList<featureTagData> deniedFeatureTags) {
            mExecutor.execute(()->{
                onDelegateCreated(status, deniedFeatureTags);
            });
        }

        public final void handleDelegateDestroyed(int status, int reason) {
            //TO have translator of reason code
            mExecutor.execute(()->{
                onDelegateDestroyed(status, reason);
            });
        }

        public void onDelegateCreated(
            int status,ArrayList<featureTagData> deniedFts) {}
        public void onDelegateDestroyed(int status, int reason) {}
        public void onEventReceived(int connectionStatus) {}
        public void onEventReceived_1_1(vendor.qti.ims.rcssip.V1_1.eventData data) {}
        public void onEventReceived_1_2(eventData data) {}
        public void onIncomingMessageReceived(ArrayList<Byte> sipMsg) {}
        public void onConnectionCmdStatus(int status, int userData) {}
        public void onFeatureTagStatusChange(
            ArrayList<featureTagData> featureTagStatus) {}
    };

}
