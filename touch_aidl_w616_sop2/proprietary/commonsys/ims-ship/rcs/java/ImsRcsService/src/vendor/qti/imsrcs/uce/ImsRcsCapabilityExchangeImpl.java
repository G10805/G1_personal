/**********************************************************************
 * Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/


package vendor.qti.imsrcs.uce;

import android.net.Uri;
import android.telephony.ims.ImsException;
import android.telephony.ims.RcsContactUceCapability;
import android.telephony.ims.stub.CapabilityExchangeEventListener;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase;
import android.util.Log;
import android.util.Pair;

import vendor.qti.imsrcs.uce.hidl.ImsPresCapEventListener;
import vendor.qti.imsrcs.uce.hidl.OptionsServiceWrapper;
import vendor.qti.imsrcs.uce.hidl.PresenceServiceWrapper;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Set;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Executor;

import vendor.qti.imsrcs.ImsRcsServiceMgr;
import vendor.qti.imsrcs.ImsRcsService;

public class ImsRcsCapabilityExchangeImpl extends RcsCapabilityExchangeImplBase {

    int mSlotId;
    CapabilityExchangeEventListener mCapExEventListener;
    Executor mExecutor;
    PresenceListener mPresListener;
    OptionsListener mOptionsListener;
    String LOG_TAG = ImsRcsService.LOG_TAG + ":ImsRcsCapabilityExchangeImpl";
    int mAospPublishTriggerType = -1;
    boolean isServiceAvable = false;
    private static final long lAvalabilityFetchMask = 0x1;

    public ImsRcsCapabilityExchangeImpl(Executor localexe, CapabilityExchangeEventListener l, int slotId) {
        super();
        mExecutor = localexe;
        mCapExEventListener = l;
        mSlotId = slotId;
        LOG_TAG = LOG_TAG +"["+mSlotId+"]";
        mPresListener = new PresenceListener(mExecutor);
        mOptionsListener = new OptionsListener(mExecutor);
        PresenceServiceWrapper presence = ImsRcsServiceMgr.getInstance().getImsPresenceService(mSlotId);
        presence.setCapInfolistener(mPresListener);
        OptionsServiceWrapper options = ImsRcsServiceMgr.getInstance().getImsOptionsService(mSlotId);
        options.setOptionsCapListeners(mOptionsListener);
    }

    @Override
    public void publishCapabilities(String pidfXml,
            PublishResponseCallback publishResponseCallback) {
        mExecutor.execute(()->{
            long userdata = mPresListener.getUserData(publishResponseCallback);
            Log.d(LOG_TAG, "publishCapabilities " + userdata);
            PresenceServiceWrapper presence = ImsRcsServiceMgr.getInstance().getImsPresenceService(mSlotId);
            presence.publishCapabilities(pidfXml, userdata);
        });
    }

    @Override
    public void sendOptionsCapabilityRequest(Uri uri,
        Set<String> list, OptionsResponseCallback optionsResponseCallback) {
        mExecutor.execute(()->{
            OptionsServiceWrapper options = ImsRcsServiceMgr.getInstance().getImsOptionsService(mSlotId);
            options.sendCapabilityRequest(uri,list,mOptionsListener.setOptionsRespCb(optionsResponseCallback));
        });

    }

    @Override
    public void subscribeForCapabilities(Collection<Uri> list,
            SubscribeResponseCallback subscribeResponseCallback) {
        mExecutor.execute(()->{
            long userdata = mPresListener.getUserData(subscribeResponseCallback, (list.size()==1));
            Log.d(LOG_TAG, "subscribeForCapabilities " + userdata);
            PresenceServiceWrapper presence = ImsRcsServiceMgr.getInstance().getImsPresenceService(mSlotId);
            presence.subscribeForCapabilities(list, userdata);
        });
    }

    private void respondToIncomingOptions(int tId, int code, String phrase,
                                           List<String> caps, boolean isContactinBlackList) {
        mExecutor.execute(()->{
            Log.d(LOG_TAG, "respondToIncomingOptions for TxId:" +tId + " code:"+code );
            OptionsServiceWrapper options = ImsRcsServiceMgr.getInstance().getImsOptionsService(mSlotId);
            options.respondToIncomingOptions(tId,code,phrase,caps,isContactinBlackList);
        });
    }

    public void checkAndNotifyPublishTrigger() {
        if(mAospPublishTriggerType != -1) {
            try {
                Log.d(LOG_TAG, "Notify fw of Publish Trigger["+mAospPublishTriggerType+"]");
                mCapExEventListener.onRequestPublishCapabilities(mAospPublishTriggerType);
            } catch (ImsException e) {
                Log.e(LOG_TAG, "Unable to send Publish Trigger");
            }
        }
    }

    private class PresenceListener extends ImsPresCapEventListener {
        int magicNum = -1;

        public PresenceListener(Executor e) {
            super(e);
        }

        @Override
        public void onServiceStatus(int s) {
            isServiceAvable = (s==0);
            if(isServiceAvable && mAospPublishTriggerType != -1) {
                try {
                    //TimerTask;
                    try {
                    Thread.sleep(200);
                    }catch(InterruptedException e){
                        e.printStackTrace();
                    }
                    mCapExEventListener.onRequestPublishCapabilities(mAospPublishTriggerType);
                } catch (ImsException e) {
                    Log.e(LOG_TAG, "Unable to send Publish Trigger");
                }
            }
        }

        @Override
        public void onRequestPublishCapabilities(int aospPublishTriggerType) {
            if(isServiceAvable) {
                try {
                    mCapExEventListener.onRequestPublishCapabilities(aospPublishTriggerType);
                } catch (ImsException e) {
                    Log.e(LOG_TAG, "Unable to send Publish Trigger");
                }
            } else {
                mAospPublishTriggerType = aospPublishTriggerType;
            }
        }

        @Override
        public void onUnpublish() {
            try {
                mCapExEventListener.onUnpublish();
            } catch (ImsException e) {
                Log.e(LOG_TAG, "Unable to send Unpublish Indication");
            }
        }

        HashMap<Long, RcsCapabilityExchangeImplBase.PublishResponseCallback> publishCbList =
                new HashMap<Long, RcsCapabilityExchangeImplBase.PublishResponseCallback>();
        HashMap<Long, RcsCapabilityExchangeImplBase.SubscribeResponseCallback> subscribeCbList =
                new HashMap<Long, RcsCapabilityExchangeImplBase.SubscribeResponseCallback>();
        Date date = new Date();

        public long getUserData(RcsCapabilityExchangeImplBase.PublishResponseCallback cb) {
            long userData = date.getTime() + (magicNum++);
            publishCbList.put(userData, cb);
            return userData;
        }
        public long getUserData(RcsCapabilityExchangeImplBase.SubscribeResponseCallback cb,
                               boolean isAvailabilityFetch) {
            long userData = date.getTime() + (magicNum++);
            userData = userData << 1;
            if(isAvailabilityFetch) {
                userData |= lAvalabilityFetchMask;
            }
            subscribeCbList.put(userData, cb);
            return userData;
        }

        private boolean isReqAvailabilityFetch(long userData) {
            long data = userData & lAvalabilityFetchMask;
            return (data > 0);
        }
        @Override
        public void onCmdStatusError(long userdata, int i) {
            try {
                if(publishCbList.get(userdata) != null) {
                    publishCbList.get(userdata).onCommandError(i);

                    publishCbList.remove(userdata);
                    return;
                }
                if(subscribeCbList.get(userdata) != null) {
                    subscribeCbList.get(userdata).onCommandError(i);
                    subscribeCbList.remove(userdata);
                    return;
                }
            } catch (ImsException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onSipResponse(long userdata, int code, String ReasonPhrase, String reasonHeader, int retryAfter) {
            Log.d(LOG_TAG, "onSipResponse " + userdata);
            try {
                if(publishCbList.get(userdata) != null) {
                    Log.d(LOG_TAG, "onSipResponse found cb");
                    if(reasonHeader.length() >0){
                        Pair<Integer,String> reasonHeaderDetails = parsetReasonHeader(reasonHeader);
                        publishCbList.get(userdata).onNetworkResponse(
                                code,ReasonPhrase,reasonHeaderDetails.first,
                                reasonHeaderDetails.second);
                    } else {
                        publishCbList.get(userdata).onNetworkResponse(code, ReasonPhrase);
                    }
                    return;
                }
                publishCbList.remove(userdata);
                if(subscribeCbList.get(userdata) != null) {
                    Log.d(LOG_TAG, "onSipResponse found cb");
                    if(reasonHeader.length() >0){
                        Pair<Integer,String> reasonHeaderDetails = parsetReasonHeader(reasonHeader);
                        subscribeCbList.get(userdata).onNetworkResponse(
                                code,ReasonPhrase,reasonHeaderDetails.first,
                                reasonHeaderDetails.second);
                        //@Note: assumption, if reason header is present meaning transaction failed
                        subscribeCbList.remove(userdata);
                    } else {
                        subscribeCbList.get(userdata).onNetworkResponse(code, ReasonPhrase);
                    }
                    return;
                }
            } catch (ImsException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onPublishUpdate(long userdata, int code, String ReasonPhrase, String reasonHeader) {
            Log.d(LOG_TAG, "onPublishUpdate " + userdata);
            try {
                Pair<Integer,String> reasonHeaderDetails = parsetReasonHeader(reasonHeader);
                mCapExEventListener.onPublishUpdated(
                        code, ReasonPhrase, reasonHeaderDetails.first,
                        reasonHeaderDetails.second);
            } catch (ImsException e) {
                Log.e(LOG_TAG, "Unable to send onPublishUpdate Indication");
            }
        }

        private Pair<Integer,String> parsetReasonHeader(String reasonHeader) {
            int result = 0;
            String resultText ="";
            if(reasonHeader.length() >0) {
                //@Example: Reason: SIP ;cause=600 ;text="Busy Everywhere"
                String[] reasons = reasonHeader.split(";");
                for (String data : reasons) {
                    if (data.contains("cause=")) {
                        String[] cause = data.split("=");
                        result = Integer.valueOf(cause[1]);
                    }
                    if (data.contains("text=")) {
                        String[] text = data.split("=");
                        resultText = text[1];
                    }
                }
            }
            return new Pair<Integer, String>(result, resultText);
        }

        @Override
        public void onNotifyCapabilitiesUpdate(long userdata, List<String> PidfXmls) {
            Log.d(LOG_TAG, "onNotifyCapabilitiesUpdate " + userdata);
            if(subscribeCbList.get(userdata) != null) {
                Log.d(LOG_TAG, "onNotifyCapabilitiesUpdate found cb");
                try {
                    subscribeCbList.get(userdata).onNotifyCapabilitiesUpdate(PidfXmls);
                } catch (ImsException e) {
                    e.printStackTrace();
                }
            }

        }

        @Override
        public void onResourceTerminated(long userdata, List<Pair<Uri, String>> uriTerminatedReason) {
            if(isReqAvailabilityFetch(userdata)) {
                //dont send this indication for Availabilty fetch
                return;
            }
            if(subscribeCbList.get(userdata) != null) {
                try {
                    subscribeCbList.get(userdata).onResourceTerminated(uriTerminatedReason);
                } catch (ImsException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onTerminated(long userdata, String reason, long retryAfterMilliseconds) {
            if(subscribeCbList.get(userdata) != null) {
                try {
                    subscribeCbList.get(userdata).onTerminated(reason,retryAfterMilliseconds);
                } catch (ImsException e) {
                    e.printStackTrace();
                }
            }
            subscribeCbList.remove(userdata);
        }

    };

    private class OptionsListener extends OptionsServiceWrapper.ImsOptionsCapEventListener   {
        int majikNum = -1;
        public OptionsListener(Executor e) {
            super(e);
        }
        HashMap<Long, OptionsResponseCallback> mOptionsRespCb = new HashMap<Long, OptionsResponseCallback>();
        public long setOptionsRespCb(OptionsResponseCallback cb) {
            Date date = new Date();
            long userData = date.getTime()+ (majikNum++);
            mOptionsRespCb.put(userData, cb);
            return userData;
        }
        @Override
        public void onRemoteCapabilityRequest(int tid, String uri, List<String> remoteFTs) {
            Log.d(LOG_TAG, "OptionsListener: incoming Options :: received TxId:" +tid);
            try {
                mCapExEventListener.onRemoteCapabilityRequest(Uri.parse(uri),
                new HashSet<String>(remoteFTs) , new OptionsNetworkCallback(tid));
            } catch (ImsException e) {
                Log.e(LOG_TAG, "Unable to send onRemoteCapabilityRequest Indication");
            }
        }

        @Override
        public void onCmdStatus(long userdata,int i) {
            Log.d(LOG_TAG, "OptionsListener: onCmdStatus :: received userdata:" +userdata+ "status:"+i);
            try {
                mOptionsRespCb.get(userdata).onCommandError(i);
            } catch (ImsException e) {
                e.printStackTrace();
            }
            mOptionsRespCb.remove(userdata);
        }

        @Override
        public void onSipResponse(long userdata,int sipCode, String reason, List<String> caps) {
            Log.d(LOG_TAG, "OptionsListener: onSipResponse :: received userdata:" +userdata+ " sipCode:"+sipCode);
            try {
                mOptionsRespCb.get(userdata).onNetworkResponse(sipCode,reason,caps);
            } catch (ImsException e) {
                e.printStackTrace();
            }
            mOptionsRespCb.remove(userdata);
        }
    };

    private class OptionsNetworkCallback implements
            CapabilityExchangeEventListener.OptionsRequestCallback {
        int mTransactionId;
        public OptionsNetworkCallback(int tid) {
            mTransactionId = tid;
        }
        @Override
        public void onRespondToCapabilityRequest(RcsContactUceCapability caps, boolean isBlackList) {
            Set<String> fts = caps.getFeatureTags();
            respondToIncomingOptions(mTransactionId, 200, "OK", new ArrayList<String>(fts), isBlackList);
        }

        @Override
        public void onRespondToCapabilityRequestWithError(int i, String s) {
            List<String> fts = new ArrayList<String>();
            respondToIncomingOptions(mTransactionId, i, s, fts, false);
        }
    };
}
