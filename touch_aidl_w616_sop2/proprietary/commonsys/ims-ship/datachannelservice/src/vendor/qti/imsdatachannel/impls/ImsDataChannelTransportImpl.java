/**********************************************************************
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.impls;

import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;

import vendor.qti.ims.datachannelservice.IDataChannelTransportListener;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelConnection;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelResponse;
import vendor.qti.imsdatachannel.base.ImsDataChannelConnectionBase;
import vendor.qti.imsdatachannel.base.ImsDataChannelTransportBase;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelTransport;
import vendor.qti.imsdatachannel.wrappers.ImsDataChannelTransportWrapper;
import vendor.qti.imsdatachannel.ImsDataChannelServiceMgr;
import vendor.qti.imsdatachannel.aidl.IImsDataChannelEventListener;
import vendor.qti.imsdatachannel.impls.ImsDataChannelConnectionImpl;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelAttributes;
import vendor.qti.imsdatachannel.aidl.ImsReasonCode;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelErrorCode;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelResponse;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelCommandErrorCode;

import vendor.qti.ims.datachannelservice.IDataChannelTransport;
import vendor.qti.ims.datachannelservice.IDataChannelConnection;
import vendor.qti.ims.datachannelservice.DataChannelResponse;
import vendor.qti.ims.datachannelservice.DataChannelCommandErrorCode;
import vendor.qti.ims.datachannelservice.DataChannelErrorCode;
import vendor.qti.ims.datachannelservice.DataChannelInfo;
import vendor.qti.ims.datachannelservice.IDataChannelConnection;

import android.os.RemoteException;
import android.util.Log;
import android.util.SparseArray;

import com.android.internal.annotations.VisibleForTesting;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.HashMap;
import java.util.*;
import android.os.IBinder;


import java.util.ArrayList;
import java.util.List;
//import vendor.qti.ims.SparseArray;

import android.content.Context;
import android.util.Log;
import android.os.Parcelable;
import vendor.qti.imsdatachannel.ImsDataChannelService;


public class ImsDataChannelTransportImpl extends ImsDataChannelTransportBase {

    private static final String LOG_TAG = ImsDataChannelService.LOG_TAG + "_ImsDataChannelTransportImpl";
    private Executor mTransportExecutor;
    ImsDataChannelServiceImpl mImsDcServiceImpl;
    private int mSlotId;
    private String mCallId;

    ImsDataChannelTransportWrapper mImsDcTransportWrapper = null;
    IImsDataChannelEventListener mImsDataChannelEventListener;
    ImsDataChannelTransportCbListener mImsDcTransportListener;
    /* map of dcId and ConnectionImpl objects */
    Map<String, ImsDataChannelConnectionBase> mImsDcConnectionImplMap = new HashMap<>();
    Vector<String> dcHandles = new Vector<String>();

    private String mDcTransportHandle = null;

    public ImsDataChannelTransportImpl(int slotId, String callId, ImsDataChannelServiceImpl dcServiceImpl, IImsDataChannelEventListener l){
        mSlotId = slotId;
        mCallId = callId;
        mTransportExecutor = new ScheduledThreadPoolExecutor(1);
        mImsDcServiceImpl = dcServiceImpl;
        mImsDcTransportListener = new ImsDataChannelTransportCbListener(mTransportExecutor);
        mImsDcTransportListener.setImsDcTransportImplInst(this);
        mImsDcTransportWrapper = new ImsDataChannelTransportWrapper(slotId,mImsDcTransportListener);
        mImsDataChannelEventListener = l;
        Log.i(LOG_TAG, "ImsDataChannelTransportImpl");
    }

    public int getSlotId() {
        return mSlotId;
    }

    public String getCallId() {
       // return Integer.parseInt(mCallId);
        return mCallId;
    }

    public void setTransportListener(IImsDataChannelEventListener l) {
        Log.i(LOG_TAG, "setTransportListener. Existing listener, Updated Listener" + mImsDataChannelEventListener + l);
        mImsDataChannelEventListener = l;
    }

    public ImsDataChannelTransportWrapper getImsDcTransportWrapperObj() {
        return mImsDcTransportWrapper;
    }

    public String getDcTransportHandle(){
        return mDcTransportHandle;
    }

    public boolean isActive() {
        return !mImsDcConnectionImplMap.isEmpty();
    }

    @Override
    public void onCreateDataChannel(String[] channelIdList, String mXmlContent){
        if(mTransportExecutor == null)
            return;
        mTransportExecutor.execute(() -> {
            Log.i(LOG_TAG, "onCreateDataChannel ");
            mImsDcTransportWrapper.createDataChannelConnections(channelIdList, mXmlContent);
        });
    }

    @Override
    public void onRespondToDataChannelSetUpRequest(ImsDataChannelResponse[] r, String mXmlContent){
        if(mTransportExecutor == null)
            return;

        Log.i(LOG_TAG, "onRespondToDataChannelSetUpRequest ");
        mTransportExecutor.execute(() -> {
            handleDataChannelSetUpResponse(r, mXmlContent);
        });
    }

    @Override
    public void onCloseDataChannel(IImsDataChannelConnection[] dc, ImsDataChannelErrorCode code){
        if(mTransportExecutor == null)
            return;
        mTransportExecutor.execute(() -> {
            String[] dcIdList = getDcIdForBinderConnections(dc);
            Log.d(LOG_TAG, "onCloseDataChannel dcIdListSize: " + dcIdList.length + "Connections listsize:" + dc.length);
            closeDataChannelConnections(dcIdList, code.getImsDataChannelErrorCode());
        });
    }

    // Helper function also used by ImsDataChannelConnectionCbListener
    public void closeDataChannelConnections(String[] dcIdList, int code) {
        mImsDcTransportWrapper.closeDataChannelConnections(dcIdList, code);
    }

    public void handleServiceDied() {
        if (mImsDcTransportListener != null) {
            mTransportExecutor.execute(() -> {
                mImsDcTransportWrapper.setAidlDcTransport(null);
                // notify remaining connections closed too
                for (ImsDataChannelConnectionBase connBase : mImsDcConnectionImplMap.values()) {
                    ImsDataChannelConnectionImpl connImpl = (ImsDataChannelConnectionImpl)connBase;
                    connImpl.handleServiceDied();
                }
                Log.d(LOG_TAG, "notifyTransportClosedToClients for transport inst:" + this.toString());
                mImsDcTransportListener.onDataChannelTransportClosed();
            });
        }
    }

    // To prevent DeadObjectException
    public void handleClientDied() {
        for (ImsDataChannelConnectionBase connBase : mImsDcConnectionImplMap.values()) {
            ImsDataChannelConnectionImpl connImpl = (ImsDataChannelConnectionImpl)connBase;
            connImpl.handleClientDied();
        }
        mImsDataChannelEventListener = null;
    }

    public IDataChannelTransport getAidlWrapperObj(){
        return mImsDcTransportWrapper.getAidlDcTransport();
    }

    public String[] getDcIdForBinderConnections(IImsDataChannelConnection[] dcBinderArr){
        List<String> dcIdList = new ArrayList();

        for (IImsDataChannelConnection imsConnBinder: dcBinderArr) {
            for (ImsDataChannelConnectionBase connBase : mImsDcConnectionImplMap.values()) {
                if (connBase.getBinder() == imsConnBinder) {
                    ImsDataChannelConnectionImpl connImpl = (ImsDataChannelConnectionImpl)connBase;
                    Log.i(LOG_TAG, "getDcIdForBinderConnections ID:" + connImpl.getConnDcId());
                    dcIdList.add(connImpl.getConnDcId());
                    break;
                }
            }
        }

        return dcIdList.toArray(new String[0]);
    }

    public void setupImsDcConnection(DataChannelInfo dcInfo, IDataChannelConnection dcConnection){
        String dcId = dcInfo.dcId;
        ImsDataChannelConnectionBase imsDcConnImpl = new ImsDataChannelConnectionImpl(dcInfo, dcConnection, this);
        mImsDcConnectionImplMap.put(dcId,imsDcConnImpl);
        Log.i(LOG_TAG, "setupImsDcConnection connection created with dcID:" + dcId + "total conn: "+ mImsDcConnectionImplMap.size());
    }

    public ImsDataChannelConnectionImpl getConnectionForDcId(String dcId){
        return (ImsDataChannelConnectionImpl)mImsDcConnectionImplMap.get(dcId);
    }

    public String getDcIdForConnection(ImsDataChannelConnectionImpl dcConnImpl){
        return dcConnImpl.getConnDcId();
    }


    public ImsDataChannelAttributes getAidlDcAttributes(DataChannelInfo dcInfo){
        ImsDataChannelAttributes imsDcAttr = new ImsDataChannelAttributes();
        imsDcAttr.setDcId(dcInfo.dcId);
        imsDcAttr.setDataChannelStreamId(dcInfo.streamId);
        imsDcAttr.setDataChannelLabel(dcInfo.dcLabel);
        imsDcAttr.setSubProtocol(dcInfo.subProtocol);
        imsDcAttr.setMaxMessageSize(dcInfo.maxMessageSize);
        return imsDcAttr;
    }

    public ImsDataChannelAttributes[] getDcAttributesList(DataChannelInfo[] dcInfoList){
        int index = -1;
        ImsDataChannelAttributes[] dcAttrList = new ImsDataChannelAttributes[dcInfoList.length];
        for(int i=0;i<dcAttrList.length;i++){
            dcAttrList[i] = getAidlDcAttributes(dcInfoList[i]);
            index = dcHandles.indexOf(dcInfoList[i].dcId);
            if(index < 0) {
                dcHandles.add(dcInfoList[i].dcId);
            }
        }
        return dcAttrList;
    }

    public void clearConnectionImpl(String dcId){
        mImsDcConnectionImplMap.remove(dcId);
        if (mImsDcConnectionImplMap.isEmpty()) {
            mImsDcServiceImpl.onCloseDataChannelTransport(getBinder());
        }
    }

    public void cleanUp() {
        Log.d(LOG_TAG, "cleanUp() setting mImsDataChannelEventListener to null");
        mTransportExecutor = null;
        mImsDcServiceImpl = null;
        mImsDcTransportListener = null;
        mImsDcTransportWrapper = null;
        mImsDataChannelEventListener = null;
        mImsDcConnectionImplMap.clear();
    }

    public ImsDataChannelErrorCode getImsDcErrorCode(int code){
      ImsDataChannelErrorCode errCode = new ImsDataChannelErrorCode();
      errCode.setImsDataChannelErrorCode(code);
      return errCode;
    }

     public ImsDataChannelCommandErrorCode getImsDcCommandErrorCode(int code){
      ImsDataChannelCommandErrorCode errCode = new ImsDataChannelCommandErrorCode();
      errCode.setImsDataChannelCommandErrorCode(code);
      return errCode;
    }

    public IDataChannelTransportListener getAidlDcTransportListener() {
        return mImsDcTransportWrapper.getAidlDcTransportListener();
    }

    private class ImsDataChannelTransportCbListener extends ImsDataChannelTransportWrapper.ImsDataChannelTransportListener{

        private ImsDataChannelTransportImpl mImsDcTransportImpl;
        public ImsDataChannelTransportCbListener(Executor e) {
            super(e);
        }

        @Override
        protected void onDataChannelTransportCreated(String dcTransportHandle,IDataChannelTransport aidlTransportObj){
            Log.i(LOG_TAG, "onDataChannelTransportCreated for dcTransportHandle: " + dcTransportHandle + "aidlobj" + aidlTransportObj.toString() );
            mImsDcTransportImpl.setupImsDcTransport(dcTransportHandle,aidlTransportObj);

        }
        @Override
        protected void onConnectionAvailable(DataChannelInfo dcInfo, IDataChannelConnection dcConnection){
            Log.i(LOG_TAG, "onConnectionAvailable dcInfo: " + dcInfo.toString() );
            ImsDataChannelAttributes imsDcAttr = mImsDcTransportImpl.getAidlDcAttributes(dcInfo);
            mImsDcTransportImpl.setupImsDcConnection(dcInfo,dcConnection);
            if(mImsDataChannelEventListener != null){
              try{
                  Log.i(LOG_TAG, " calling onDataChannelAvailable");
              mImsDcTransportImpl.mImsDataChannelEventListener.onDataChannelAvailable(imsDcAttr,mImsDcTransportImpl.getConnectionForDcId(dcInfo.dcId).getBinder());
              } catch(RemoteException e){
                e.printStackTrace();
              }
            }

        }

        @Override
        protected void onConnectionCreated(DataChannelInfo dcInfo, IDataChannelConnection dcConnection){
            ImsDataChannelAttributes imsDcAttr = mImsDcTransportImpl.getAidlDcAttributes(dcInfo);
            mImsDcTransportImpl.setupImsDcConnection(dcInfo,dcConnection);
            if(mImsDataChannelEventListener != null){
              try{
                  Log.i(LOG_TAG, " calling onDataChannelCreated");
              mImsDcTransportImpl.mImsDataChannelEventListener.onDataChannelCreated(imsDcAttr,mImsDcTransportImpl.getConnectionForDcId(dcInfo.dcId).getBinder());
              } catch(RemoteException e){
                e.printStackTrace();
              }
            }
        }

        @Override
        protected void onConnectionSetupRequest(DataChannelInfo[] dcInfoList){
            ImsDataChannelAttributes[] dcAttrList = mImsDcTransportImpl.getDcAttributesList(dcInfoList);
            if(mImsDataChannelEventListener != null){
              try{
                  Log.i(LOG_TAG, " calling onDataChannelSetupRequest");
              mImsDcTransportImpl.mImsDataChannelEventListener.onDataChannelSetupRequest(dcAttrList);
              } catch(RemoteException e){
                e.printStackTrace();
              }
            }
        }

        @Override
        protected void onConnectionSetupError(DataChannelInfo dcInfo, int code){
            //mImsDcTransportImpl.cleanUpConnImpl(dcId);

            if(mImsDataChannelEventListener != null){
               try{
                   Log.i(LOG_TAG, " calling onDataChannelSetupError with errorCode: " + code);
                mImsDcTransportImpl.mImsDataChannelEventListener.onDataChannelSetupError(dcInfo.dcId,mImsDcTransportImpl.getImsDcErrorCode(code));
               } catch(RemoteException e){
                 e.printStackTrace();
               }
            }
              //mImsDcTransportImpl.mImsDataChannelEventListener.onDataChannelClosed(dcId,reasonCode);
        }

        @Override
        protected void onDataChannelCommandError(String dcId, int errorCode){
            if(mImsDataChannelEventListener != null){
               try{
                   Log.i(LOG_TAG, " calling onDataChannelCommandError with dcID" + dcId + "errorCode: " + errorCode);
                mImsDcTransportImpl.mImsDataChannelEventListener.onDataChannelCommandError(dcId,mImsDcTransportImpl.getImsDcCommandErrorCode(errorCode));
                } catch(RemoteException e){
                 e.printStackTrace();
               }
            }
        }

        @Override
        protected void onDataChannelTransportClosed(){

            if(mImsDataChannelEventListener != null){
               try{
                ImsReasonCode reasonCode = new ImsReasonCode();
                reasonCode.setImsReasonCode(ImsReasonCode.SUCCESS);
                mImsDcTransportImpl.mImsDataChannelEventListener.onDataChannelTransportClosed(reasonCode);
                Log.i(LOG_TAG, " calling onDataChannelTransportClosed");
               }
                catch(RemoteException e){
                 e.printStackTrace();
               }
            }
            if (mImsDcTransportImpl != null) {
                mImsDcServiceImpl.handleTransportClosed(mImsDcTransportImpl);
                mImsDcTransportImpl.cleanUp();
                mImsDcTransportImpl = null;
            }
        }

        public void setImsDcTransportImplInst(ImsDataChannelTransportImpl mImsDcTransportImpl){
            this.mImsDcTransportImpl = mImsDcTransportImpl;
        }


    }

    private void setupImsDcTransport(String dcTransportHandle, IDataChannelTransport aidlTransportObj) {
        Log.i(LOG_TAG, " calling setupImsDcTransport");
        mImsDcTransportWrapper.setAidlDcTransport(aidlTransportObj);
        this.mDcTransportHandle = dcTransportHandle;
    }

    private void handleDataChannelSetUpResponse(ImsDataChannelResponse[] resp, String mXmlContent) {

        DataChannelResponse[] dcResponseArr = new DataChannelResponse[resp.length];
        int index = -1;
        for(int i=0; i< resp.length; i++) {
            DataChannelResponse dcResp = new DataChannelResponse();
            index = dcHandles.indexOf(resp[i].getDcId());
            if(index >= 0) {
                dcHandles.remove(index);
            } else {
                if(mImsDataChannelEventListener != null){
                    try{
                        Log.i(LOG_TAG, " calling onDataChannelCommandError with dcID = " + resp[i].getDcId());
                        mImsDataChannelEventListener.onDataChannelCommandError(resp[i].getDcId(),
                        getImsDcCommandErrorCode(ImsDataChannelCommandErrorCode.DCS_COMMAND_INVALID_PARAM));
                    } catch(RemoteException e) {
                      e.printStackTrace();
                    }
                }
                return;
            }
            dcResp.dcId = resp[i].getDcId();
            dcResp.acceptStatus = resp[i].isAcceptStatus();
            dcResponseArr[i] = dcResp;
        }
        mImsDcTransportWrapper.respondToDataChannelSetupRequest(dcResponseArr, mXmlContent);
    }


}
