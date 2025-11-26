/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.dctestapp;

import android.os.Bundle;
import android.os.RemoteException;
import android.util.Log;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.Button;

import androidx.annotation.NonNull;
import androidx.fragment.app.DialogFragment;
import androidx.fragment.app.Fragment;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.Arrays;

import vendor.qti.imsdatachannel.aidl.DataChannelState;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelAttributes;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelCommandErrorCode;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelErrorCode;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelMessage;
import vendor.qti.imsdatachannel.aidl.ImsDataChannelState;
import vendor.qti.imsdatachannel.aidl.ImsMessageStatus;
import vendor.qti.imsdatachannel.aidl.ImsMessageStatusInfo;
import vendor.qti.imsdatachannel.client.ImsDataChannelConnection;
import vendor.qti.imsdatachannel.client.ImsDataChannelMessageCallback;
import vendor.qti.imsdatachannel.client.ImsDataChannelTransport;

public class DcConnectionFragment extends DialogFragment implements ImsDataChannelMessageCallback {

    private String LOG_TAG = "DCTestApp:DcConnectionFragment";

    ImsDataChannelAttributes mAttr;
    private ImsDataChannelConnection mDcConnection = null;
    private ImsDataChannelTransport mDcTransport = null;

    Executor mExecutor = new ScheduledThreadPoolExecutor(1);

    private List<String> messages = new ArrayList<String>();
    private ViewGroup msgContainer;
    private Button sendMsgButton;
    private Button closeDcConnectionButton;
    private boolean mDataChannelConnected = true;
    private int nextMsgNum = 1;
    private String dcConnectionId;
    private int maxFragmentLength = 64000;
    private boolean waitingForSendStatus = false;
    private Queue<byte[]> pendingDCMessages = new LinkedList<byte[]>();

    private File mExternalFileDir;
    final String BOOTSTRAP_MSG_FILE_NAME = "DataChannelMsg.txt";
    final String APPLICATION_MSG_FILE_NAME = "AppDataChannelMsg.txt";
    final String BOOTSTRAP_RCVD_MSG_FILE_NAME = "ReceivedBootstrapMsg.txt";
    final String APPLICATION_RCVD_MSG_FILE_NAME = "ReceivedAppMsg.txt";
    private static int bootstrapBytesWritten = 0;
    private static int appBytesWritten = 0;
    final int FILE_SIZE_LIMIT = 52428800; // 50MiB

    public DcConnectionFragment(ImsDataChannelAttributes attr, ImsDataChannelConnection dcConnection, ImsDataChannelTransport dcTransport, File externalFileDir) {
        Log.d(LOG_TAG, "attr[dcId[" + attr.getDcId() + "], streamId[" + attr.getDataChannelStreamId() + "], label[" + attr.getDataChannelLabel() + "]]");
        mAttr = attr;
        mDcConnection = dcConnection;
        mDcTransport = dcTransport;
        mExternalFileDir = externalFileDir;
        dcConnectionId = attr.getDcId() + " : " + attr.getDataChannelStreamId();
        LOG_TAG = LOG_TAG + "[" + dcConnectionId + "]";
    }

    public ImsDataChannelConnection getDcConnection() {
        return mDcConnection;
    }

    public static void resetFiles() {
        Log.d("DCTestApp:DcConnectionFragment", "Done writing to files: bootstrapBytesWritten=" + bootstrapBytesWritten + " appBytesWritten=" + appBytesWritten);
        bootstrapBytesWritten = 0;
        appBytesWritten = 0;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(
            @NonNull LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        Log.d(LOG_TAG, "onCreateView");
        View root = inflater.inflate(R.layout.fragment_dc_connection, container, false);

        TextView dataChannelLabel = root.findViewById(R.id.textView_dataChannelLabel);
        dataChannelLabel.setText(mAttr.getDataChannelLabel());

        // Present messages sent and received
        msgContainer = root.findViewById(R.id.linearLayout_dcMessageLog);
        for (String message : messages) {
            showMessage(message);
        }

        sendMsgButton = root.findViewById(R.id.button_sendMessage);
        sendMsgButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                handleSendDcMessage();
            }
        });
        sendMsgButton.setEnabled(mDataChannelConnected);

        closeDcConnectionButton = root.findViewById(R.id.button_closeDcConnection);
        closeDcConnectionButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                handleCloseDcConnection();
            }
        });

        return root;
    }

    @Override
    public void onResume() {
        super.onResume();
        ViewGroup.LayoutParams params = getDialog().getWindow().getAttributes();
        params.width = LayoutParams.MATCH_PARENT;
        params.height = LayoutParams.MATCH_PARENT;
        getDialog().getWindow().setAttributes((android.view.WindowManager.LayoutParams) params);
    }

    /*-------- Callbacks from ImsDataChannelStatusCallback --------*/

    public void onClosed(ImsDataChannelErrorCode code) {
        newMessage("DataChannel closed: code[" + code.getImsDataChannelErrorCode() + "]");
        mDataChannelConnected = false;
        if (sendMsgButton != null) {
            sendMsgButton.post(() -> {
                sendMsgButton.setEnabled(mDataChannelConnected);
            });
        }
    }

    public void onStateChange(ImsDataChannelState dcState) {
        int state = dcState.getState().getDataChannelState();
        newMessage("DataChannel state changed: " + state);
        mDataChannelConnected = state == DataChannelState.DATA_CHANNEL_CONNECTED;
        if (sendMsgButton != null) {
            sendMsgButton.post(() -> {
                sendMsgButton.setEnabled(mDataChannelConnected);
            });
        }
    }

    private void showMessage(String message) {
        if (msgContainer == null) {
            return;
        }
        msgContainer.post(() -> {
            TextView msgView = new TextView(getActivity());
            msgView.setText(message);
            msgView.setLayoutParams(new LayoutParams(LayoutParams.MATCH_PARENT,
                                                     LayoutParams.WRAP_CONTENT));
            msgView.setTextSize(14);
            msgView.setPadding(8, 8, 8, 8);
            msgContainer.addView(msgView);
            // Log.d(LOG_TAG, "showMessage(): added message: " + message);
        });
    }

    private void newMessage(String message) {
        messages.add(message);
        showMessage(message);
    }

    private void sendDataChannelMessage(byte[] bytes) {
        if (waitingForSendStatus) {
            Log.d(LOG_TAG, "sendDataChannelMessage() queueing message for later");
            pendingDCMessages.add(bytes);
        } else {
            waitingForSendStatus = true;
            Log.d(LOG_TAG, "sendDataChannelMessage()");
            ImsDataChannelMessage msg = new ImsDataChannelMessage();
            msg.setDcId(mAttr.getDcId());
            String protocolId = "51"; // TODO make configurable
            Log.d(LOG_TAG, "sendDataChannelMessage() protocolId = " + protocolId);
            msg.setProtocolId(protocolId);
            msg.setMessageId(Integer.toString(nextMsgNum));
            nextMsgNum++;
            msg.setMessage(bytes);
            Log.d(LOG_TAG, "sendDataChannelMessage() msgId=" + msg.getMessageId() + " length=" + msg.getMessage().length + " bytes");
            newMessage("Sending: message length " + msg.getMessage().length + " bytes");
            try {
                mDcConnection.sendMessage(msg);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "sendDataChannelMessage RemoteException!!!");
            }
        }
    }

    public void handleSendDcMessage() {
        Log.d(LOG_TAG, "handleSendDcMessage()");
        byte[] bytes;
        File msgFile;
        if (mAttr.getDataChannelStreamId() < 1000) {
            msgFile = new File(mExternalFileDir, BOOTSTRAP_MSG_FILE_NAME);
        } else {
            msgFile = new File(mExternalFileDir, APPLICATION_MSG_FILE_NAME);
        }
        Path msgPath = Paths.get(msgFile.getPath());
        Log.e(LOG_TAG,"handleSendDcMessage() Reading file : " + msgFile.getPath());
        try {
            bytes = Files.readAllBytes(msgPath);
        } catch (IOException e) {
            Log.e(LOG_TAG, "handleSendDcMessage IOException!!! Using dummy data");
            newMessage("Couldn't read message file: " + msgFile.getPath() + ". Sending dummy data.");
            String s = "";
            for (int i = 0; i < 35; i++) {
                s += "abc1234567890";
            }
            bytes = s.getBytes();
        }
        Log.e(LOG_TAG,"handleSendDcMessage() Total message length " + bytes.length + " bytes");
        //Check for #of bytes
        int numOfSegments = 0;
        for (int i = 0; i < bytes.length; i += maxFragmentLength) {
            int j = Math.min(bytes.length, i + maxFragmentLength);
            byte[] tempFragmentMsg = Arrays.copyOfRange(bytes, i, j);
            sendDataChannelMessage(tempFragmentMsg);
            numOfSegments++;
            Log.e(LOG_TAG,"handleSendDcMessage() Msg Sent of Length " + tempFragmentMsg.length + " Fragment Number " + numOfSegments);
        }
        Log.e(LOG_TAG,"handleSendDcMessage() Number of Fragmented Msg Sent " + numOfSegments);
    }

    private void handleCloseDcConnection() {
        Log.d(LOG_TAG, "handleCloseDcConnection()");
        ImsDataChannelConnection[] dc = new ImsDataChannelConnection[]{mDcConnection};
        ImsDataChannelErrorCode code = new ImsDataChannelErrorCode();
        try {
            mDcTransport.closeDataChannel(dc, code);
        } catch (RemoteException e) {
            Log.e(LOG_TAG, "dcTransport.closeDataChannel RemoteException!!!");
        }
        dismiss();
    }

    /*-------- Overrides for ImsDataChannelMessageCallback --------*/

    @Override
    public void onMessageReceived(ImsDataChannelMessage msg) {
        byte[] bytes = msg.getMessage();
        String s = new String(bytes, StandardCharsets.UTF_8);
        Log.d(LOG_TAG, "onMessagedReceived() length: " + bytes.length + " bytes");
        newMessage("Received: message length " + bytes.length + " bytes");

        // Write message to file up to
        File msgFile;
        int bytesWritten;
        if (mAttr.getDataChannelStreamId() < 1000) {
            msgFile = new File(mExternalFileDir, BOOTSTRAP_RCVD_MSG_FILE_NAME);
            bytesWritten = bootstrapBytesWritten;
        } else {
            msgFile = new File(mExternalFileDir, APPLICATION_RCVD_MSG_FILE_NAME);
            bytesWritten = appBytesWritten;
        }
        if (bytesWritten + bytes.length > FILE_SIZE_LIMIT) {
            Log.e(LOG_TAG, "onMessageReceived(): file size limit reached. Not writing anymore bytes: " + msgFile.getPath());
            return;
        }
        Path msgPath = Paths.get(msgFile.getPath());
        Log.d(LOG_TAG,"onMessageReceived() Writing file : " + msgFile.getPath());
        try {
            if (bytesWritten == 0) {
                // Default options truncate file to 0
                Log.d(LOG_TAG, "onMessageReceived(): File truncated before writing");
                Files.write(msgPath, bytes);
            } else {
                Files.write(msgPath, bytes, StandardOpenOption.CREATE, StandardOpenOption.APPEND, StandardOpenOption.WRITE);
            }
        } catch (IOException e) {
            Log.e(LOG_TAG, "onMessageReceived() could not write to file");
        }
        if (mAttr.getDataChannelStreamId() < 1000) {
            bootstrapBytesWritten += bytes.length;
        } else {
            appBytesWritten += bytes.length;
        }
    }

    @Override
    public void onMessageSendStatus(ImsMessageStatusInfo msgStatusInfo) {
        Log.d(LOG_TAG, "onMessageSendStatus()");
        waitingForSendStatus = false;
        int status = msgStatusInfo.getMsgStatus().getImsMessageStatus();
        String message = "Send status: " + status;
        if (status == ImsMessageStatus.MESSAGE_STATUS_FAILURE) {
            message += ", error code: " + msgStatusInfo.getMsgStatusErrorCode();
            Log.d(LOG_TAG, "onMessageSendStatus(MESSAGE_STATUS_FAILURE) clearing pending message fragments");
            pendingDCMessages.clear();
        } else if (!pendingDCMessages.isEmpty()) {
            Log.d(LOG_TAG, "onMessageSendStatus() sending one pending message fragment");
            sendDataChannelMessage(pendingDCMessages.remove());
        }
        newMessage(message);
    }

    @Override
    public void onMessageSendCommandError(ImsDataChannelCommandErrorCode errorcode) {
        Log.d(LOG_TAG, "onMessageSendCommandError()");
        newMessage("Command Error: " + errorcode.getImsDataChannelCommandErrorCode());
    }
}
