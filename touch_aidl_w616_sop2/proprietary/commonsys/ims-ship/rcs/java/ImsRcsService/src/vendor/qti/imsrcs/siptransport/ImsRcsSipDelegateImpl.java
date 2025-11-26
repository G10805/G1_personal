/**********************************************************************
 * Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **********************************************************************/

package vendor.qti.imsrcs.siptransport;

import vendor.qti.imsrcs.siptransport.hidl.SipDelegateWrapper;
import vendor.qti.imsrcs.siptransport.hidl.SipTransportServiceWrapper;
import vendor.qti.ims.rcssip.V1_0.configData;
import vendor.qti.ims.rcssip.V1_2.connectionEvent;
import vendor.qti.ims.rcssip.V1_0.deviceConfigKeys;
import vendor.qti.ims.rcssip.V1_0.featureTagData;
import vendor.qti.ims.rcssip.V1_0.featureTagRegStatusCode;
import vendor.qti.ims.rcssip.V1_0.deniedReasonCode;
import vendor.qti.ims.rcssip.V1_0.keyValuePairStringType;
import vendor.qti.ims.rcssip.V1_0.PeerInfoType;
import vendor.qti.ims.rcssip.V1_0.ReasonCode;
import vendor.qti.ims.rcssip.V1_0.messageData;
import vendor.qti.ims.rcssip.V1_0.sessionData;
import vendor.qti.ims.rcssip.V1_0.sipMessageKeys;
import vendor.qti.ims.rcssip.V1_0.SipTransportStatusCode;
import vendor.qti.ims.rcssip.V1_0.userConfigKeys;
import vendor.qti.ims.rcssip.V1_2.eventData;
import vendor.qti.ims.rcssip.V1_1.rcsServiceRatType;

import android.os.RemoteException;
import android.telephony.ims.DelegateMessageCallback;
import android.telephony.ims.DelegateRegistrationState;
import android.telephony.ims.DelegateStateCallback;
import android.telephony.ims.FeatureTagState;
import android.telephony.ims.SipDelegateImsConfiguration;
import android.telephony.ims.SipDelegateManager;
import android.telephony.ims.SipMessage;
import android.telephony.ims.ImsRegistrationAttributes;
import android.telephony.ims.stub.SipDelegate;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ims.ImsReasonInfo;
import android.util.ArraySet;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.ScheduledFuture;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Set;

import vendor.qti.imsrcs.ImsRcsService;

public class ImsRcsSipDelegateImpl implements SipDelegate {
    public ImsRcsSipTransportImpl mSipTransportServiceInstance;
    public SipTransportServiceWrapper mSipTransportWrapper;
    private static int INVALID_USERDATA = 0;
    private static int INVALID_CONFIG_VERSION = -1;

    int mCreateConnUserdata = INVALID_USERDATA;
    int mConfigVersion = INVALID_CONFIG_VERSION;
    DelegateStateCallback mSipDelegateStateCb;
    DelegateMessageCallback mSipDelegateMsgCb;
    SipDelegateWrapper mSipDelegateWrapper;
    ArrayList<String> mRequestedFtList = new ArrayList<String>();
    ArrayList<String> mDeniedFtsList = new ArrayList<String>();
    ArrayList<String> mSupportedFts = new ArrayList<String>();
    ArraySet<String> mRegisteredFts = new ArraySet<>();
    HashMap<Integer, SipMessage> mSipMessageMap = new
        HashMap<Integer, SipMessage>();
    final String LOG_TAG = "ImsRcsSipDelegateImpl";
    Executor mSipDelegateExecutor = new ScheduledThreadPoolExecutor(1);
    int mSipDelegateUserData = 4000;
    private String mOutboundProxyName;
    private String mOutboundProxyPort;
    private boolean isSipDelegateActive = true;
    private boolean isUserConfigDataAvailable = false;
    private boolean isDeviceConfigDataAvailable = false;
    public boolean isSipDelegateNeedsRestoration = false;
    private boolean isSipDelegateRestoring = false;
    private boolean isSipDelegatePendingDestroy = false;
    private ScheduledFuture<?> mCreateSipDelegateTimer = null;
    public int mDelegateDestroyReason = -1;
    SipDelegateImsConfiguration mSipDelegateConfig;

    public ImsRcsRegistrationImpl regImpl = null;

    HashMap<String, String> CallIdToFeatureTagMapping = new HashMap<String, String>();

    public ImsRcsSipDelegateImpl(
        DelegateStateCallback delegateStateCallback,
        DelegateMessageCallback delegateMessageCallback,
        ImsRcsSipTransportImpl sipTransportObj,
        SipTransportServiceWrapper sipTransportWrapperObj)
   {
        Log.d(LOG_TAG, "ImsRcsSipDelegateImpl ctor");
        mSipDelegateStateCb = delegateStateCallback;
        mSipDelegateMsgCb = delegateMessageCallback;
        mSipTransportServiceInstance = sipTransportObj;
        mSipTransportWrapper = sipTransportWrapperObj;
        mSipDelegateWrapper = new SipDelegateWrapper();
        mSipDelegateWrapper.setSipDelegateEventListener(mSipDelegateListener);
        regImpl = ImsRcsService.getImsRcsRegistrationImpl(
            mSipTransportServiceInstance.mSlotId);
    }

    public void setDelegateWrapper(SipDelegateWrapper delegateWrapper) {
        mSipDelegateWrapper = delegateWrapper;
        mSipDelegateWrapper.setSipDelegateEventListener(mSipDelegateListener);
    }

    public void setCreateConnectionUserData(int userData) {
        mCreateConnUserdata = userData;
    }

    public int getCreateConnectionUserData() {
        return mCreateConnUserdata;
    }

    public void setRequestedFeatureTagsList(ArrayList<String> featureTags) {
        mRequestedFtList.addAll(featureTags);
    }

    public ArrayList<String> getRequestedFeatureTagsList() {
        return mRequestedFtList;
    }

    public ArrayList<String> getRequestedFeatureTags() {
        if(mRequestedFtList.size() > 0)
           Log.d(LOG_TAG, "getRequestedFeatureTags RequestedFts exists");
        return mRequestedFtList;
    }

    public void setDeniedFtsList(ArrayList<String> featureTags) {
        mDeniedFtsList.addAll(featureTags);
    }

    public ArrayList<String> getDeniedFtsList() {
        return mDeniedFtsList;
    }

    private void setOutboundProxyName(String value) {
        mOutboundProxyName = value;
    }

    public String getOutboundProxyName() {
        return mOutboundProxyName;
    }

    private void setOutboundProxyPort(String value) {
        mOutboundProxyPort = value;
    }

    public String getOutboundProxyPort() {
        return mOutboundProxyPort;
    }

    private void setSupportedFtList(ArrayList<String> ftList) {
        Log.i(LOG_TAG, ":setSupportedFtList");
        mSupportedFts.addAll(ftList);
        for(String ft: mSupportedFts)
        {
            Log.d(LOG_TAG, ":setSupportedFtList: added fts: "+ ft);
        }
    }

    public ArrayList<String> getSupportedFtList() {
        return mSupportedFts;
    }

    public void setIsDelegateActive(boolean isActive) {
        isSipDelegateActive = isActive;
    }

    public void setIsDelegateNeedstoRestore(boolean status) {
        isSipDelegateNeedsRestoration = status;
    }

    public boolean getIsDelegateNeedstoRestore() {
        return isSipDelegateNeedsRestoration;
    }

    public void setIsDelegateRestoring(boolean status) {
        isSipDelegateRestoring = status;
    }

    public boolean getIsDelegateRestoring() {
        return isSipDelegateRestoring;
    }

    public void setIsDelegatePendingDestroy(boolean status, int reason) {
        isSipDelegatePendingDestroy = status;
        mDelegateDestroyReason = reason;
    }

    public boolean getIsDelegatePendingDestroy() {
        return isSipDelegatePendingDestroy;
    }

    public void setCreateSipDelegateTimer(ScheduledFuture<?> scheduledTimer) {
        mCreateSipDelegateTimer = scheduledTimer;
    }

    public boolean isCreateSipDelegateRequestPending() {
        if((isSipDelegateActive == true) && (mCreateConnUserdata == INVALID_USERDATA))
            return true;

        return false;
    }

   public boolean isConnectionHandleValid() {
       if(mSipDelegateWrapper.getHidlConnectionHandle() != 0)
            return true;

        return false;
   }

    public void cancelCreateSipDelegateTimer() {
        if(mCreateSipDelegateTimer != null) {
            Log.d(LOG_TAG, ": cancelCreateSipDelegateTimer");
            mCreateSipDelegateTimer.cancel(true);
        }
    }

    @Override
    public void cleanupSession(String callId) {
        mSipDelegateUserData++;
        Log.d(LOG_TAG, ": closeDialog called");
        mSipDelegateWrapper.handleCloseTransaction(
            callId,
            mSipDelegateUserData);
    }

    @Override
    public void notifyMessageReceiveError(String viaTransactionId,
                                          int reason) {
        //No handling required
    }

    @Override
    public void notifyMessageReceived(String viaTransactionId) {
        //No handling required
    }

    @Override
    public void sendMessage(SipMessage sipMessage, long configVersion) {
        Log.i(LOG_TAG,"sendMessage API called");
        if(mConfigVersion != configVersion) {
            Log.i(LOG_TAG,"sendMessage::Invoking onMessageSendFailure"+
                          " reason: stale config");
            mSipDelegateMsgCb.onMessageSendFailure(
                    sipMessage.getViaBranchParameter(),
                    SipDelegateManager
                    .MESSAGE_FAILURE_REASON_STALE_IMS_CONFIGURATION);
        } else{
            Log.d(LOG_TAG,"sendMessage::config version matched");
            mSipDelegateUserData++;
            mSipMessageMap.put(mSipDelegateUserData, sipMessage);
            ImsSipMessageParser msgParser = new ImsSipMessageParser(
                sipMessage);

            //TODO: should we have some check before
            //setting data in each field below
            messageData msg = new messageData();

            keyValuePairStringType outboundProxyData = new
               keyValuePairStringType();
            outboundProxyData.key = sipMessageKeys.OutboundProxy;
            outboundProxyData.value = msgParser.getOutboundProxy();
            msg.data.add(outboundProxyData);

            keyValuePairStringType remotePortData = new
                keyValuePairStringType();
            remotePortData.key = sipMessageKeys.RemotePort;
            remotePortData.value = Integer.toString(
                msgParser.getRemotePort());
            msg.data.add(remotePortData);

            keyValuePairStringType protocolData = new
                keyValuePairStringType();
            protocolData.key = sipMessageKeys.Protocol;
            protocolData.value = Integer.toString(msgParser.getProtocol());
            msg.data.add(protocolData);

            keyValuePairStringType messageTypeData = new
                keyValuePairStringType();
            messageTypeData.key = sipMessageKeys.MessageType;
            messageTypeData.value = Integer.toString(
                msgParser.getMessageType());
            msg.data.add(messageTypeData);

            keyValuePairStringType callIdData = new keyValuePairStringType();
            callIdData.key = sipMessageKeys.CallId;
            callIdData.value = msgParser.getCallID();
            msg.data.add(callIdData);


            byte[] messageByteArray = sipMessage.toEncodedMessage();
            for(int i = 0; i < messageByteArray.length; i++) {
                msg.bufferData.add(messageByteArray[i]);
            }
            Log.i(LOG_TAG,"sendMessage copied buffersize "+
                  Integer.toString(messageByteArray.length)+
                  "  final buffersize : "+
                  Integer.toString(msg.bufferData.size()));

            mSipDelegateWrapper.handleSendMessage(msg, mSipDelegateUserData);
            updateSessionStatus(msgParser, PeerInfoType.PEER_LOCAL);
        }
    }

    public void updateUserAgentToFw(String userAgentValue){
        Log.d(LOG_TAG,"updateUserAgentToFw called");
        SipDelegateImsConfiguration.Builder c;

        if(mSipDelegateConfig != null){
            ++mConfigVersion;
            c = new SipDelegateImsConfiguration.Builder(mSipDelegateConfig);
            c.addString(SipDelegateImsConfiguration.KEY_SIP_CONFIG_USER_AGENT_HEADER_STRING,
                   mSipTransportServiceInstance.getUserAgentValue());
            mSipDelegateConfig = c.build();
            Log.d(LOG_TAG,"Sending onImsConfigurationChanged");
            if(isUserConfigDataAvailable && isDeviceConfigDataAvailable) {
              mSipDelegateStateCb.onImsConfigurationChanged(mSipDelegateConfig);
            }
        }
    }
    public void onConfigurationChange(configData configData) {
        Log.d(LOG_TAG,"onConfigurationChange called");

        SipDelegateImsConfiguration.Builder c;
        if(mSipDelegateConfig == null) {
            c = new SipDelegateImsConfiguration.Builder(++mConfigVersion);
        } else {
            c = new SipDelegateImsConfiguration.Builder(mSipDelegateConfig);
            ++mConfigVersion;
        }

        //set userAgent header
        String userAgent = mSipTransportServiceInstance.getUserAgentValue();
        c.addString(SipDelegateImsConfiguration.KEY_SIP_CONFIG_USER_AGENT_HEADER_STRING,
                   mSipTransportServiceInstance.getUserAgentValue());

        //for now setting to TCP
        c.addString(
            SipDelegateImsConfiguration.KEY_SIP_CONFIG_TRANSPORT_TYPE_STRING,
            SipDelegateImsConfiguration.SIP_TRANSPORT_TCP);
        if(configData.userConfigData != null &&
           configData.userConfigData.size()>0)
        {
            isUserConfigDataAvailable = true;
            for(int i =0; i < configData.userConfigData.size(); i++) {
                keyValuePairStringType var = configData.userConfigData.get(i);
                fillImsConfig(c, var.key, var.value);
            }
        } else {
            Log.d(LOG_TAG,"onConfigurationChange:no userConfigKeys in this iter");
        }

        if(configData.deviceConfigData != null &&
           configData.deviceConfigData.size()>0)
        {
            isDeviceConfigDataAvailable = true;
            for(int i =0; i < configData.deviceConfigData.size(); i++) {
                keyValuePairStringType var = configData
                    .deviceConfigData.get(i);
                fillImsConfig(c, var.key, var.value);
            }
        } else {
            Log.d(LOG_TAG,"onConfigurationChange:no deviceConfigKeys in this iter");
        }

        mSipDelegateConfig = c.build();
        Log.d(LOG_TAG,"Config version before posting to FW: "+
                      mConfigVersion);

        if(isUserConfigDataAvailable && isDeviceConfigDataAvailable) {
            Log.d(LOG_TAG,"Sending onImsConfigurationChanged");
            mSipDelegateStateCb.onImsConfigurationChanged(mSipDelegateConfig);
        }
    }

    String updateImeiString(String imeiStringVal) {
        int imeiStringLength = imeiStringVal.length();
        StringBuffer newString= new StringBuffer(imeiStringVal);
        Log.d(LOG_TAG," updateImeiString: string length : "+
                       Integer.toString(imeiStringLength));
        if(imeiStringLength> 18 || imeiStringLength<1) {
            Log.d(LOG_TAG," updateImeiString: improper string length");
        } else if(imeiStringLength == 15){
            // done as per modem
            newString.insert(8, "-");
            newString.insert(15, "-");
        }
        return newString.toString();
    }

    //Note:KEY_SIP_CONFIG_UE_DEFAULT_PORT_INT is missing right now
    //Should we determine transport type based on threshold value
    private void fillImsConfig(
        SipDelegateImsConfiguration.Builder c,
        int key,
        String value) {
        switch(key) {
            case userConfigKeys.UEClientPort:
                c.addInt(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_UE_IPSEC_CLIENT_PORT_INT,
                        Integer.parseInt(value));
                break;
            case userConfigKeys.UEServerPort:
                c.addInt(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_UE_IPSEC_SERVER_PORT_INT,
                        Integer.parseInt(value));

                c.addInt(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_UE_DEFAULT_PORT_INT,
                        Integer.parseInt(value));
                break;
            case userConfigKeys.AssociatedURI:
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_P_ASSOCIATED_URI_HEADER_STRING,
                        value);
                break;
            case userConfigKeys.UEPublicIPAddress:
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_UE_PUBLIC_IPADDRESS_WITH_NAT_STRING,
                        value);
                break;
            case userConfigKeys.UEPublicPort:
                c.addInt(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_UE_PUBLIC_PORT_WITH_NAT_INT,
                        Integer.parseInt(value));
               break;
            case userConfigKeys.SipPublicUserId:
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_UE_PUBLIC_USER_ID_STRING,
                        value);
               break;
            case userConfigKeys.SipPrivateUserId:
               c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_UE_PRIVATE_USER_ID_STRING,
                        value);
                break;
            case userConfigKeys.SipHomeDomain:
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_HOME_DOMAIN_STRING,
                        value);
                break;
            case userConfigKeys.UEPubGruu:
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_UE_PUBLIC_GRUU_STRING,
                        value);
                break;
            case userConfigKeys.LocalHostIPAddress:
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_UE_DEFAULT_IPADDRESS_STRING,
                        value);
                break;
            case userConfigKeys.IpType:
                String ipTypeValue = "";
                if(value.equals("1"))
                {
                  ipTypeValue = SipDelegateImsConfiguration.IPTYPE_IPV4;
                }
                else if(value.equals("2"))
                {
                  ipTypeValue = SipDelegateImsConfiguration.IPTYPE_IPV6;
                }
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_IPTYPE_STRING,
                        ipTypeValue);
                break;
            case userConfigKeys.IMEIStr:
                String updatedValue = updateImeiString(value);
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_IMEI_STRING,
                        updatedValue);
                break;
            case userConfigKeys.UEOldSAClientPort:
                c.addInt(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_UE_IPSEC_OLD_CLIENT_PORT_INT,
                        Integer.parseInt(value));
                break;
            case deviceConfigKeys.UEBehindNAT:
                c.addBoolean(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_IS_NAT_ENABLED_BOOL,
                        value.equals("1"));
                break;
            case deviceConfigKeys.IpSecEnabled:
                c.addBoolean(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_IS_IPSEC_ENABLED_BOOL,
                        value.equals("1"));
                break;
            case deviceConfigKeys.CompactFormEnabled:
                c.addBoolean(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_IS_COMPACT_FORM_ENABLED_BOOL,
                        value.equals("1"));
                break;
            case deviceConfigKeys.KeepAliveEnableStatus:
                c.addBoolean(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_IS_KEEPALIVE_ENABLED_BOOL,
                        value.equals("1"));
                break;
            case deviceConfigKeys.GruuEnabled:
                c.addBoolean(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_IS_GRUU_ENABLED_BOOL,
                        value.equals("1"));
                break;
            case deviceConfigKeys.StrSipOutBoundProxyName:
                setOutboundProxyName(value);
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_SERVER_DEFAULT_IPADDRESS_STRING,
                        value);
                break;
            case deviceConfigKeys.SipOutBoundProxyPort:
                setOutboundProxyPort(value);
                c.addInt(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_SERVER_DEFAULT_PORT_INT,
                        Integer.parseInt(value));
                break;
            case deviceConfigKeys.PCSCFClientPort:
                c.addInt(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_SERVER_IPSEC_CLIENT_PORT_INT,
                        Integer.parseInt(value));
                break;
            case deviceConfigKeys.PCSCFServerPort:
                c.addInt(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_SERVER_IPSEC_SERVER_PORT_INT,
                        Integer.parseInt(value));
                break;
            case deviceConfigKeys.ArrAuthChallenge:
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_AUTHENTICATION_HEADER_STRING,
                        value);
                break;
            case deviceConfigKeys.ArrNC:
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_AUTHENTICATION_NONCE_STRING,
                        value);
                break;
            case deviceConfigKeys.ServiceRoute:
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_SERVICE_ROUTE_HEADER_STRING,
                        value);
                break;
            case deviceConfigKeys.SecurityVerify:
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_SECURITY_VERIFY_HEADER_STRING,
                        value);
                break;
            case deviceConfigKeys.PCSCFOldSAClientPort:
                c.addInt(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_SERVER_IPSEC_OLD_CLIENT_PORT_INT,
                        Integer.parseInt(value));
                break;
            case deviceConfigKeys.TCPThresholdValue:
                // TOCHECK: how this can help in deciding
                // the transport type is tcp or udp
                c.addInt(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_MAX_PAYLOAD_SIZE_ON_UDP_INT,
                        Integer.parseInt(value));
                break;
            case deviceConfigKeys.PANI:
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_P_ACCESS_NETWORK_INFO_HEADER_STRING,
                        value);
                break;
            case deviceConfigKeys.PATH:
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_PATH_HEADER_STRING,
                        value);
                break;
            case deviceConfigKeys.UriUserPart:
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_URI_USER_PART_STRING,
                        value);
                break;
            case deviceConfigKeys.PLANI:
                c.addString(
                    SipDelegateImsConfiguration
                        .KEY_SIP_CONFIG_P_LAST_ACCESS_NETWORK_INFO_HEADER_STRING,
                        value);
                break;
            default:
                break;
        }
    }

    ImsRcsSipDelegateImpl.SipDelegateListener mSipDelegateListener =
        new SipDelegateListener(mSipDelegateExecutor);

    //use this as a wrapper for AOSP callback as well
    private class SipDelegateListener extends
        SipDelegateWrapper.ImsSipDelegateEventListener {

        public SipDelegateListener(Executor e) {
            super(e);
        }

        @Override
        public void onDelegateCreated(
            int status,
            ArrayList<featureTagData> deniedFts) {
            ArraySet<FeatureTagState> featureTagStateArraySet = new
                ArraySet<>();
            ArrayList<String> deniedFtList = new ArrayList<String>();
            //default value
            int ftDataState = SipDelegateManager.DENIED_REASON_UNKNOWN;

            for(featureTagData ftData : deniedFts) {
                //Currently in our native we are filling
                //all Fts as one String with one state
                //we should parse that in java layer;
                //hence calling the below converter fn
                deniedFtList.addAll(convertFtStringtoList(ftData.featureTag));
            }

            for(String deniedFt : deniedFtList) {
                FeatureTagState ftState = new
                    FeatureTagState(deniedFt, ftDataState);
                featureTagStateArraySet.add(ftState);
            }
            setDeniedFtsList(deniedFtList);
            setSupportedFeatureTags();
            if(!(isSipDelegateRestoring)) {
                mSipDelegateStateCb.onCreated(
                    ImsRcsSipDelegateImpl.this,
                    featureTagStateArraySet);
            } else {
                //Reset the restoration flag back to false after
                //connection is successfully restored
                Log.d(LOG_TAG,"onDelegateCreated: Restored Delegate; not sending onCreated to FW");
                isSipDelegateRestoring = false;
                if(!getIsDelegatePendingDestroy())
                {
                    //Calling TriggerReg manually since FW cant call
                    //updateSipDelegateRegistration()
                    //without receiving onCreated cb.
                    if(mSipTransportWrapper != null) {
                        Log.d(LOG_TAG,"onDelegateCreated: calling TriggerRegistration");
                        int userData = mSipTransportWrapper.triggerRegistration();
                        if(regImpl != null)
                          regImpl.setUserData(userData);
                    }else{
                        Log.d(LOG_TAG,"onDelegateCreated: mSipTransportWrapper is null");
                    }
                } else {
                    Log.d(LOG_TAG,"onDelegateCreated: handling pending destroyDelegate");
                    setIsDelegateActive(false);
                    if(isConnectionHandleValid()) {
                        if(mSipTransportWrapper != null) {
                           mSipTransportWrapper.closeConnection(
                              mDelegateDestroyReason,
                              mSipDelegateWrapper);
                        }
                    }
                    //reset flag
                    setIsDelegatePendingDestroy(false, -1);
                }
            }
        }

        private ArrayList<String> convertFtStringtoList(String featureTag) {
            String botVersionStr = "botversion";
            ArrayList<String> ftTags = new ArrayList<String>();

            if(!(featureTag.isEmpty())) {
                ftTags.addAll(Arrays.asList(featureTag.split(";")));
            }
            return ftTags;
        }

        @Override
        public void onDelegateDestroyed(int status, int reason) {
            Log.i(LOG_TAG, ": onDelegateDestroyed with reason: "+
                  Integer.toString(reason));
            if(mSipTransportServiceInstance != null)
                mSipTransportServiceInstance.mSipDelegateList.remove(
                        ImsRcsSipDelegateImpl.this);
            int AOSP_reason = convertHidlDestroyedReason(reason);
            DelegateRegistrationState.Builder delegateRegState =
                new DelegateRegistrationState.Builder();
            for(String deregisteredFt: mSupportedFts) {
                delegateRegState.addDeregisteredFeatureTag(
                    deregisteredFt,
                    DelegateRegistrationState.DEREGISTERED_REASON_NOT_REGISTERED);
            }
            DelegateRegistrationState regState = delegateRegState.build();
            mSipDelegateStateCb.onFeatureTagRegistrationChanged(regState);
            ImsRcsRegistrationImpl regImpl = ImsRcsService.getImsRcsRegistrationImpl(
                mSipTransportServiceInstance.mSlotId);
            if (regImpl != null) {
              if ((regImpl.getRegisteredFeatureTags().size()) > 0)
              {
                regImpl.clearRegisteredFeatureTags();
                ImsReasonInfo info = new ImsReasonInfo(
                  ImsReasonInfo.CODE_UNSPECIFIED, 0);
                Log.i(LOG_TAG, ": Invoking onDeregistered with reason: CODE_UNSPECIFIED ");
                regImpl.onDeregistered(info);
              }
            } else {
                Log.d(LOG_TAG, ":getImsRcsRegistrationImpl returned NULL obj");
            }
            if(!(mRegisteredFts.isEmpty())) {
                Log.i(LOG_TAG, ": onDelegateDestroyed clearing RegFts");
                mRegisteredFts.clear();
            }
            mSipDelegateStateCb.onDestroyed(AOSP_reason);
        }

        @Override
        public void onEventReceived(int connectionStatus) {
            Log.i(LOG_TAG, ": onEventReceived: called: connectionStatus: "+
                           Integer.toString(connectionStatus));

            if(connectionEvent.SERVICE_NOTREGISTERED == connectionStatus) {
                handleNotRegisterEvent(null);
            }
            else if (connectionEvent.SERVICE_REGISTERED == connectionStatus) {
                ArraySet<String> featuresList = handleRegisterEvent();
                if (regImpl != null) {
                    int aospRegistrationTech = regImpl.getRatType();
                    regImpl.checkToInvokeOnRegistered(featuresList, aospRegistrationTech);
                } else {
                    Log.d(LOG_TAG, "getImsRcsRegistrationImpl returned null");
                }
            }
        }

        @Override
        public void onEventReceived_1_1(vendor.qti.ims.rcssip.V1_1.eventData evtData) {
            Log.i(LOG_TAG, ": onEventReceived_1_1: called: connectionStatus: "+
                           Integer.toString(evtData.event));

            if(connectionEvent.SERVICE_NOTREGISTERED == evtData.event) {
                handleNotRegisterEvent(evtData.causeCode);
            } else if (connectionEvent.SERVICE_REGISTERED == evtData.event) {
                ArraySet<String> featuresList = handleRegisterEvent();
                ArrayList<String> registeredFtList = new ArrayList<String>(featuresList);
                for(String ft: registeredFtList) {
                    Log.d(LOG_TAG, "onEventReceived_1_1: registered fts: " + ft);
                    mRegisteredFts.add(ft);
                }
                int aospRegistrationTech = convertHidlRatTypeToAospRegTech(evtData.ratType);
                Log.d(LOG_TAG, "ImsRcsRegistrationImpl call onRegistered");
                if(regImpl != null)
                    regImpl.checkToInvokeOnRegistered(mRegisteredFts, aospRegistrationTech);
                }
        }

        @Override
        public void onEventReceived_1_2(eventData evtData) {
            Log.i(LOG_TAG, ": onEventReceived_1_2: called: connectionStatus: "+
                           Integer.toString(evtData.event));
            if (regImpl != null) {
                regImpl.removeRegisteringFeatureTags(new ArraySet(mSupportedFts));
            }

            if(connectionEvent.SERVICE_NOTREGISTERED == evtData.event) {
                handleNotRegisterEvent(evtData.causeCode);
            } else if (connectionEvent.SERVICE_REGISTERED == evtData.event) {
                ArraySet<String> featuresList = handleRegisterEvent();
                ArrayList<String> registeredFtList = new ArrayList<String>(featuresList);
                for(String ft: registeredFtList) {
                    Log.d(LOG_TAG, "onEventReceived_1_2: registered fts: " + ft);
                    mRegisteredFts.add(ft);
                }
                int aospRegistrationTech = convertHidlRatTypeToAospRegTech(evtData.ratType);
                Log.d(LOG_TAG, "ImsRcsRegistrationImpl call onRegistered");
                if(regImpl != null)
                    regImpl.checkToInvokeOnRegistered(mRegisteredFts, aospRegistrationTech);

            } else if(connectionEvent.SERVICE_REGISTERING == evtData.event) {
                if(regImpl != null && isSipDelegateActive && mSupportedFts.size() > 0) {
                    regImpl.setRegisteringFeatureTags(new ArraySet(mSupportedFts));
                    regImpl.postOnRegistering();
                }
            }
        }

        @Override
        public void onFeatureTagStatusChange(
            ArrayList<featureTagData> featureTagList) {
            if(featureTagList == null) return;

            DelegateRegistrationState.Builder delegateRegState =
                     new DelegateRegistrationState.Builder();
            ArraySet<String> features = new ArraySet<>();

            for(featureTagData featureTagInfo: featureTagList) {
                if(featureTagInfo.state ==connectionEvent
                    .SERVICE_REGISTERED) {
                    delegateRegState.addRegisteredFeatureTag(
                        featureTagInfo.featureTag);
                    features.addAll(convertFtStringtoList(featureTagInfo.featureTag));
                    ImsRcsRegistrationImpl regImpl = ImsRcsService.getImsRcsRegistrationImpl(
                    mSipTransportServiceInstance.mSlotId);
                    if (regImpl != null) {
                      Log.d(LOG_TAG, "ImsRcsRegistrationImpl call onRegistered");
                      int aospRegistrationTech = regImpl.getRatType();
                      regImpl.checkToInvokeOnRegistered(features, aospRegistrationTech);
                    } else {
                        Log.d(LOG_TAG, "getImsRcsRegistrationImpl returned NULL object");
                    }
                } else if(featureTagInfo.state == connectionEvent
                    .SERVICE_NOTREGISTERED) {
                    delegateRegState.addDeregisteredFeatureTag(
                        featureTagInfo.featureTag,
                        DelegateRegistrationState
                            .DEREGISTERED_REASON_UNKNOWN);
                } else if(featureTagInfo.state >= featureTagRegStatusCode
                    .UNKNOWN) {
                    delegateRegState.addDeregisteredFeatureTag(
                        featureTagInfo.featureTag,
                        convertHidlConnectionStatus(featureTagInfo.state));
                } else {
                    return;
                }
            }
            if(regImpl != null) {
                regImpl.removeRegisteringFeatureTags(features);
            }
            mSipDelegateStateCb.onFeatureTagRegistrationChanged(
                delegateRegState.build());
        }

        @Override
        public void onIncomingMessageReceived(ArrayList<Byte> sipMsg) {
            Log.i(LOG_TAG, ":onIncomingMessageReceived: size:["+
                           sipMsg.size()+"]");
            byte[] sipBytes = new byte[sipMsg.size()];
            for (int i = 0; i < sipMsg.size(); i++) {
                sipBytes[i] = sipMsg.get(i);
            }

            String strSipMsg = new String(sipBytes);
            Log.i(LOG_TAG, ":onIncomingMessageReceived: strSipMsg : "+
                           strSipMsg);
            ImsSipMessageParser msgParser = new ImsSipMessageParser (strSipMsg);

            Log.i(LOG_TAG, ":onIncomingMessageReceived: startLine :"
                  +msgParser.getStartLine());
            Log.i(LOG_TAG, ":onIncomingMessageReceived: HeaderSection :"
                  +msgParser.getHeaderSection());
            Log.i(LOG_TAG, ":onIncomingMessageReceived: MessageContent :"
                  +msgParser.getMessageContent());
            if(msgParser.getMessageContent() == null) {
              Log.d(LOG_TAG, ":onIncomingMessageReceived:"+
                             "msgcontent is null, but forwarding message");
              SipMessage incomingMsg = new SipMessage(
                                                  msgParser.getStartLine(),
                                                  msgParser.getHeaderSection(),
                                                  new byte[0]);
              mSipDelegateMsgCb.onMessageReceived(incomingMsg);
            }
            else {
                SipMessage incomingMsg = new SipMessage(
                    msgParser.getStartLine(),
                    msgParser.getHeaderSection(),
                    msgParser.getMessageContent().getBytes());
                mSipDelegateMsgCb.onMessageReceived(incomingMsg);
            }
            updateSessionStatus(msgParser, PeerInfoType.PEER_REMOTE);
        }

        @Override
        public void onConnectionCmdStatus(int status, int userData) {
            Log.d(LOG_TAG, "onConnectionCmdStatus called");
            if(status == SipTransportStatusCode.FAILURE) {
                if(mSipMessageMap.containsKey(userData)) {
                    mSipDelegateMsgCb.onMessageSendFailure(
                        mSipMessageMap.get(userData).getViaBranchParameter(),
                        SipDelegateManager
                            .MESSAGE_FAILURE_REASON_NETWORK_NOT_AVAILABLE);
                    mSipMessageMap.remove(userData);
                }
            } else if (status == SipTransportStatusCode.SUCCESS) {
                if(mSipMessageMap.containsKey(userData)) {
                    mSipDelegateMsgCb.onMessageSent(
                        mSipMessageMap.get(userData).getViaBranchParameter());
                    mSipMessageMap.remove(userData);
                }
                else if (regImpl.getUserData() == userData) {
                    Log.d(LOG_TAG, "ImsRcsRegistrationImpl call onRegistering");
                    ImsRegistrationAttributes attr = new ImsRegistrationAttributes.Builder(
                        ImsRegistrationImplBase.REGISTRATION_TECH_NONE).build();
                    regImpl.onRegistering(attr);
                }
            }
        }

        private int convertHidlConnectionStatus(int status) {
            switch(status) {
                case (int) featureTagRegStatusCode.NOT_PROVISIONED:
                    return DelegateRegistrationState
                        .DEREGISTERED_REASON_NOT_PROVISIONED;
                case (int) featureTagRegStatusCode.NOT_REGISTERED:
                    return DelegateRegistrationState
                        .DEREGISTERED_REASON_NOT_REGISTERED;
                default:
                    return DelegateRegistrationState
                        .DEREGISTERED_REASON_UNKNOWN;
            }
        }

        private int convertHidlDestroyedReason(int reason) {
            if(ReasonCode.UNKNOWN == reason) {
                return SipDelegateManager.SIP_DELEGATE_DESTROY_REASON_UNKNOWN;
            } else if(ReasonCode.SERVICE_DEAD == reason) {
                return SipDelegateManager
                    .SIP_DELEGATE_DESTROY_REASON_SERVICE_DEAD;
            } else if(ReasonCode.REQUESTED_BY_APP == reason) {
                return SipDelegateManager
                    .SIP_DELEGATE_DESTROY_REASON_REQUESTED_BY_APP;
            } else if(ReasonCode.USER_DISABLED_RCS == reason) {
                return SipDelegateManager
                    .SIP_DELEGATE_DESTROY_REASON_USER_DISABLED_RCS;
            } else if(ReasonCode.SUBSCRIPTION_TORN_DOWN == reason) {
                return SipDelegateManager
                    .SIP_DELEGATE_DESTROY_REASON_SUBSCRIPTION_TORN_DOWN;
            } else {
                return SipDelegateManager
                    .SIP_DELEGATE_DESTROY_REASON_UNKNOWN;
            }
        }
    }

    private int getDeniedReason(int deniedReason) {
        if(SipTransportStatusCode.INVALID_PARAM == deniedReason) {
            return  SipDelegateManager.DENIED_REASON_INVALID ;
        } else {
            return  SipDelegateManager.DENIED_REASON_UNKNOWN;
        }
    }

    private void setSupportedFeatureTags() {
        String ChatbotFt =
            "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application"+
            ".ims.iari.rcs.chatbot\"";
        String ChatbotSaFt =
            "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application"+
            ".ims.iari.rcs.chatbot.sa\"";
        List<String>  deniedBotVersions;
        List<String> requestedBotVersions = null;
        ArrayList<String> requestedTags = getRequestedFeatureTags();
        ArrayList<String> deniedTags = getDeniedFtsList();
        String botVersionString = "+g.gsma.rcs.botversion=\"";
        int botVersionStandardSize = botVersionString.length();
        Log.d(LOG_TAG, ": setSupportedFeatureTags called");
        //Calculates the difference between requested and denied fts
        requestedTags.removeAll(deniedTags);

        //Calculate the botversions in deniedFt
        //if chatbot/chatbot.sa is present
        deniedBotVersions = calculateBotVersionsFromFt(deniedTags);

        if(deniedBotVersions.size() > 0) {
            Log.d(LOG_TAG, ": setSupportedFeatureTags - "+
                           "deniedBotVersions found in response");
            requestedBotVersions = calculateBotVersionsFromFt(requestedTags);
        }
        if( requestedBotVersions != null )
        {
            List<String> mSupportedBotVersions = new
                ArrayList<>(requestedBotVersions);
            mSupportedBotVersions.removeAll(deniedBotVersions);

            if(mSupportedBotVersions.size()>0) {
                for(int i =0; i<mSupportedBotVersions.size(); i++) {
                    if(i != 0)
                        botVersionString = botVersionString.concat(",");
                    botVersionString = botVersionString
                        .concat("#=")
                        .concat(mSupportedBotVersions.get(i));
                    if(i == (mSupportedBotVersions.size()-1))
                        botVersionString = botVersionString.concat("\"");
                    Log.d(LOG_TAG, ": setSupportedFeatureTags - "+
                                   "botVersionString: " +botVersionString);
                }
            }

            Log.d(LOG_TAG, ": setSupportedFeatureTags - "+
                           "full botVersionString: " +botVersionString);

            if(botVersionString.length() > botVersionStandardSize) {
                // making temp list to avoid ConcurrentModificationException
                ArrayList<String> tempFtList = new ArrayList<>(requestedTags);

                for(String ft : tempFtList)
                {
                    if(ft.contains("botversion")) {
                        if(requestedTags.contains(ChatbotFt) ||
                           requestedTags.contains(ChatbotSaFt)) {
                            requestedTags.add(botVersionString);
                        }
                        requestedTags.remove(ft);
                    }
                }
            }
        }

        for(String ftString: requestedTags) {
            Log.i(LOG_TAG, ": setSupportedFeatureTags - "+
                           "supportedFts: " +ftString);
        }
        setSupportedFtList(requestedTags);
    }

    private List<String> calculateBotVersionsFromFt(
        ArrayList<String> ftList) {
        String[] botVersions = new String[0];
        for(String ft: ftList) {
            if(ft.contains("botversion")) {
                String[] tempVersions = ft.split("botversion=\"");
                String v = tempVersions[1];
                v = v.replaceAll("\"","");
                v = v.replaceAll("#=","");
                botVersions = v.split(",");
            }
        }
        //TODO:remove duplicate entries
        return Arrays.asList(botVersions);
    }

    private void updateSessionStatus(ImsSipMessageParser msgParser, int peerInfo)
    {
        Log.d(LOG_TAG, ": updateSessionStatus");
        if(msgParser.getSdpContent().isEmpty() == false)
        {
            Log.d(LOG_TAG, ": updateSessionStatus - msg has sdp body");
            sessionData sessData = new sessionData();
            sessData.peer = peerInfo;
            sessData.data = msgParser.getSdpContent();

            ArrayList<sessionData> sessionVec = new
                ArrayList<sessionData>(1);
            sessionVec.add(sessData);
            Log.d(LOG_TAG, ": updateSessionStatus - before calling"+
                            " native updateSessionContent");
            mSipDelegateWrapper.handleUpdateSession(msgParser.getCallID(), sessionVec);
            checkRcsSessionStatus(msgParser);
        }
   }

    private ArraySet<String> handleRegisterEvent() {
        Log.i(LOG_TAG,": handleRegEvent:SERVICE_REGISTERED evt: ");
        DelegateRegistrationState.Builder delegateRegState =
            new DelegateRegistrationState.Builder();
        ArraySet<String> features = new ArraySet<>();
        for(String ft: mSupportedFts) {
            Log.d(LOG_TAG, "supported fts: " + ft);
            delegateRegState.addRegisteredFeatureTag(ft);
            features.add(ft);
        }

        DelegateRegistrationState regState = delegateRegState.build();
        mSipDelegateStateCb.onFeatureTagRegistrationChanged(regState);

        return features;
    }

    private void handleNotRegisterEvent(String reasonCode)
    {
        Log.i(LOG_TAG,": handleNotRegEvent:SERVICE_NOTREGISTERED evt: ");
        DelegateRegistrationState.Builder delegateRegState =
            new DelegateRegistrationState.Builder();
        for(String deregisteredFt: mSupportedFts) {
            delegateRegState.addDeregisteredFeatureTag(
                deregisteredFt,
                DelegateRegistrationState
                    .DEREGISTERED_REASON_NOT_REGISTERED);
        }

        DelegateRegistrationState regState = delegateRegState.build();
        mSipDelegateStateCb.onFeatureTagRegistrationChanged(regState);
        int reasonInfo = ImsReasonInfo.CODE_UNSPECIFIED;

        if(regImpl != null) {
            if(reasonCode != null) {
                if(reasonCode.equals("RCS_CONNECTION_CAUSE_CODE_NONE"))
                    reasonInfo = ImsReasonInfo.CODE_UNSPECIFIED;
                else if(reasonCode.equals("RCS_CONNECTION_CAUSE_CODE_GENERIC_ERROR"))
                    reasonInfo = ImsReasonInfo.CODE_LOCAL_NOT_REGISTERED;
                ImsReasonInfo info = new ImsReasonInfo(reasonInfo, 0);
                mRegisteredFts.clear();
                regImpl.onDeregistered(info);
            } else {
                //Indicates V1_0
                if ((regImpl.getRegisteredFeatureTags().size()) > 0)
                {
                    regImpl.clearRegisteredFeatureTags();
                    ImsReasonInfo info = new ImsReasonInfo(reasonInfo, 0);
                    Log.i(LOG_TAG, ": Invoking onDeregistered with reason: CODE_UNSPECIFIED ");
                    regImpl.onDeregistered(info);
                }
            }
        }else {
            Log.d(LOG_TAG, " getImsRcsRegistrationImpl returned NULL obj");
        }
    }

    private int convertHidlRatTypeToAospRegTech(int ratType)
    {
        int aospRegistrationTech = ImsRegistrationImplBase.REGISTRATION_TECH_NONE;
        if(ratType == rcsServiceRatType.RCS_RAT_LTE) {
            Log.d(LOG_TAG, " rcsServiceRatType.RCS_RAT_LTE");
            aospRegistrationTech = ImsRegistrationImplBase.REGISTRATION_TECH_LTE;
        } else if (ratType == rcsServiceRatType.RCS_RAT_NR5G) {
            Log.d(LOG_TAG, " rcsServiceRatType.RCS_RAT_NR5G");
            aospRegistrationTech = ImsRegistrationImplBase.REGISTRATION_TECH_NR;
        } else if (ratType == rcsServiceRatType.RCS_SERVICE_RAT_IWLAN) {
            Log.d(LOG_TAG, " rcsServiceRatType.RCS_SERVICE_RAT_IWLAN");
            aospRegistrationTech = ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN;
        } else if (ratType == rcsServiceRatType.RCS_SERVICE_RAT_C_IWLAN) {
            Log.d(LOG_TAG, " rcsServiceRatType.RCS_SERVICE_RAT_C_IWLAN");
            aospRegistrationTech = ImsRegistrationImplBase.REGISTRATION_TECH_CROSS_SIM;
        } else {
            Log.d(LOG_TAG, " none of case match; setting to REGISTRATION_TECH_NONE");
        }

        return aospRegistrationTech;
   }

   private void checkRcsSessionStatus(ImsSipMessageParser msgParser) {
       Log.d(LOG_TAG,"inside checkRcsSessionStatus");

                if(msgParser.isRequestMessage() && msgParser.isInviteMessage()) {
                    Log.d(LOG_TAG, " msgParser is requestMEssage and is also invite message");
                    CallIdToFeatureTagMapping.put(msgParser.getCallID(),
                                                msgParser.getAcceptContactFt());
                } else if(msgParser.isResponseMessage()) {
                    Log.d(LOG_TAG, " msgParser is response message");
                    if(msgParser.isInviteMessage() && CallIdToFeatureTagMapping.containsKey(msgParser.getCallID())) {
                        Log.d(LOG_TAG, "callid also matched");
                         //check active or inactive and call HAL setStatus api
                          mSipDelegateUserData++;
                          if(msgParser.isSessionStatusActive()) {
                              Log.d(LOG_TAG, " session active");
                            mSipDelegateWrapper.handleActiveSessionStatus(CallIdToFeatureTagMapping.get(msgParser.getCallID()), mSipDelegateUserData);
                          } else if(msgParser.isSessionStatusHold()) {
                              Log.d(LOG_TAG, "session on hold");
                            mSipDelegateWrapper.handleHoldSessionStatus(CallIdToFeatureTagMapping.get(msgParser.getCallID()), mSipDelegateUserData);
                          }else {
                              Log.d(LOG_TAG, " session unknown");
                          }
                    } else if(msgParser.isByeMessage() && CallIdToFeatureTagMapping.containsKey(msgParser.getCallID())) {
                        mSipDelegateUserData++;
                        mSipDelegateWrapper.handleInactiveSessionStatus(CallIdToFeatureTagMapping.get(msgParser.getCallID()), mSipDelegateUserData);
                        CallIdToFeatureTagMapping.remove(msgParser.getCallID());
                    } else {
                        Log.d(LOG_TAG,"looks like callid did not match");
                    }
                }

   }

}
