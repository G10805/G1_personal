/**********************************************************************
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsdatachannel.wrappers;
import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import vendor.qti.imsdatachannel.ImsDataChannelService;
import vendor.qti.ims.datachannelservice.IDataChannelConnection;
import vendor.qti.ims.datachannelservice.DataChannelInfo;
import vendor.qti.ims.datachannelservice.DataChannelErrorCode;
import vendor.qti.ims.datachannelservice.DataChannelCommandErrorCode;
import vendor.qti.ims.datachannelservice.DataChannelResponse;
import vendor.qti.ims.datachannelservice.DataChannelErrorCode;
import vendor.qti.ims.datachannelservice.IDataChannelTransport;
import vendor.qti.ims.datachannelservice.IDataChannelTransportListener;
import android.os.IBinder;
import java.util.*;
import android.os.ServiceManager;
import android.util.Log;
import android.os.RemoteException;
import android.os.Parcelable;

public class ImsDataChannelTransportWrapper{

    private static final String LOG_TAG = ImsDataChannelService.LOG_TAG + "_ImsDataChannelTransportWrapper";
    private IDataChannelTransport mAidlDcTransport = null;
    private int slotId;
    private DataChannelTransportListener mAidlDcTransportListener;
    private ImsDataChannelTransportListener mImsDcTransportListener;

    public ImsDataChannelTransportWrapper(int slotId,ImsDataChannelTransportListener mImsDcTransportListener){
       this.slotId = slotId;
       mAidlDcTransportListener = new DataChannelTransportListener();
       this.mImsDcTransportListener = mImsDcTransportListener;
    }

    public void setAidlDcTransport(IDataChannelTransport mAidlDcTransport){
        this.mAidlDcTransport = mAidlDcTransport;
    }

    public IDataChannelTransport getAidlDcTransport(){
        return mAidlDcTransport;
    }

    public DataChannelTransportListener getAidlDcTransportListener(){
        return mAidlDcTransportListener;
    }


    public void createDataChannelConnections(String[] channelIdList, String mXmlContent){
        try{
            if(mAidlDcTransport == null){
             Log.e(LOG_TAG, "mAidlDcTransport is null for createDataChannelConnections ");
             return;
            }
            mAidlDcTransport.createDataChannelConnections(channelIdList,mXmlContent);
        } catch(RemoteException e){
            e.printStackTrace();
        }
    }

    public void respondToDataChannelSetupRequest(DataChannelResponse[] r, String mXmlContent){
        try{
            if(mAidlDcTransport == null){
              Log.e(LOG_TAG, "mAidlDcTransport is null for respondToDataChannelSetupRequest ");
              return;
            }
            mAidlDcTransport.respondToDataChannelSetupRequest(r,mXmlContent);
        } catch(RemoteException e){
            e.printStackTrace();
        }
    }

    public void closeDataChannelConnections(String[] dcIdList, int dcErrorCode){
        try{
            //DataChannelErrorCode dcCode = new DataChannelErrorCode();
            if(mAidlDcTransport == null){
              Log.e(LOG_TAG, "mAidlDcTransport is null for closeDataChannelConnections ");
              return;
            }
            mAidlDcTransport.closeDataChannelConnections(dcIdList, dcErrorCode);
        } catch(RemoteException e){
            e.printStackTrace();
        }
    }


    private class DataChannelTransportListener extends IDataChannelTransportListener.Stub{
        private static final String LOG_TAG = ImsDataChannelTransportWrapper.LOG_TAG + ".IDataChannelTransportListener";

        @Override
        public void onDataChannelTransportCreated(String dcTransportHandle, IDataChannelTransport iDataChannelTransport) throws RemoteException {
            if(mImsDcTransportListener != null){
                Log.i(LOG_TAG, " recd onDataChannelTransportCreated ");
                mImsDcTransportListener.handleOnDataChannelTransportCreated(dcTransportHandle,iDataChannelTransport);
            }
        }

        @Override
        public void onConnectionAvailable(DataChannelInfo dcInfo, IDataChannelConnection dcConnection){
        if(mImsDcTransportListener != null){
            Log.i(LOG_TAG, " recd onConnectionAvailable ");
            mImsDcTransportListener.handleOnConnectionAvailable(dcInfo,dcConnection);
        }
        }

        @Override
        public void onConnectionCreated(DataChannelInfo dcInfo, IDataChannelConnection dcConnection){
            if(mImsDcTransportListener != null){
                mImsDcTransportListener.handleOnConnectionCreated(dcInfo,dcConnection);
            }

        }

        @Override
        public void onConnectionSetupRequest(DataChannelInfo[] dcInfoList){
            if(mImsDcTransportListener != null){
                mImsDcTransportListener.handleOnConnectionSetupRequest(dcInfoList);
            }

        }

        @Override
        public void onConnectionSetupError(DataChannelInfo dcInfo, int code){
            if(mImsDcTransportListener != null){
                mImsDcTransportListener.handleOnConnectionSetupError(dcInfo,code);
            }
        }


        @Override
        public void onDataChannelCommandError(String dcId, int errorCode){
            if(mImsDcTransportListener != null){
                Log.i(LOG_TAG, " calling onDataChannelCommandError with errorCode: " + errorCode);
                mImsDcTransportListener.handleOnDataChannelCommandError(dcId,errorCode);
            }

        }

        @Override
        public void onDataChannelTransportClosed(){
            if(mImsDcTransportListener != null){
                mImsDcTransportListener.handleOnDataChannelTransportClosed();
            }
        }

        @Override
        public final int getInterfaceVersion() { return IDataChannelTransportListener.VERSION; }

        @Override
        public final String getInterfaceHash() { return IDataChannelTransportListener.HASH; }
    }


    public static abstract class ImsDataChannelTransportListener{
        protected Executor mExecutor;

        public ImsDataChannelTransportListener(Executor e){
            this.mExecutor = e;
        }

        public void handleOnDataChannelTransportCreated(String dcTransportHandle, IDataChannelTransport iDataChannelTransport) {
            mExecutor.execute(
                    () -> {
                        onDataChannelTransportCreated(dcTransportHandle,iDataChannelTransport);
                    }
            );
        }

        public final void handleOnConnectionAvailable(DataChannelInfo dcInfo, IDataChannelConnection dcConnection){
           mExecutor.execute(
            () -> {
                onConnectionAvailable(dcInfo,dcConnection);
            }
           );
        }

        public final void handleOnConnectionCreated(DataChannelInfo dcInfo, IDataChannelConnection dcConnection){
            mExecutor.execute(
                () -> {
                    onConnectionCreated(dcInfo,dcConnection);
                }
               );
        }


        public final void handleOnConnectionSetupRequest(DataChannelInfo[] dcInfoList){
            mExecutor.execute(
                () -> {
                    onConnectionSetupRequest(dcInfoList);
                }
               );

        }

        public final void handleOnConnectionSetupError(DataChannelInfo dcInfo, int code){
            mExecutor.execute(
                () -> {
                    onConnectionSetupError(dcInfo,code);
                }
               );
        }

        public final void handleOnDataChannelCommandError(String dcId, int errorCode){
            mExecutor.execute(
                () -> {
                    onDataChannelCommandError(dcId,errorCode);
                }
               );

        }

        public final void handleOnDataChannelTransportClosed(){
            mExecutor.execute(
                () -> {
                    onDataChannelTransportClosed();
                }
               );

        }

        protected void onDataChannelTransportCreated(String dcTransportHandle, IDataChannelTransport iDataChannelTransport) {
        }
        protected void onConnectionAvailable(DataChannelInfo dcInfo, IDataChannelConnection dcConnection){

        }
        protected void onConnectionCreated(DataChannelInfo dcInfo, IDataChannelConnection dcConnection){

        }
        protected void onConnectionSetupRequest(DataChannelInfo[] dcInfoList){

        }

        protected void onConnectionSetupError(DataChannelInfo dcInfo, int code){

        }

        protected void onDataChannelCommandError(String dcId, int errorCode){}

        protected void onDataChannelTransportClosed(){}


    }




}
