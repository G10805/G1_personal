/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
package com.qualcomm.qti.locationqesdkdemo.tp1.ntrip;

import android.annotation.SuppressLint;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.time.Instant;
import java.util.Base64;
import java.lang.Math;
import java.util.Date;

public class NtripClientSocket {

    private final NtripClient.LogIface logger;

    String mHostName;
    String mMountPoint;
    String mUserNamePwdEncodedInBase64Format;
    String mGGANmea = "$GNGGA,,,,,,,,,,,,,,*48\r\n";
    static final String NTRIP_1_0_ACK = "ICY 200 OK";
    static final String NTRIP_2_0_ACK = "HTTP/1.1 200 OK";
    int mPort;
    int mMaxContentDataBufferSizeInBytes;
    int mTimeoutForFullBufferInSec;
    int mSocketReadRetryAttempts;
    int mWaitTimeBetweenReconnectAttemptsInSec;
    boolean mUseNtrip2Version;
    byte[] mDataContentBuffer;
    NtripClient.CorrectionDataCallback mCorrectionDataCb;
    boolean mLogCorrectionData;
    int mPacketCount;
    int mSocketFd;
    boolean mStopRequested;
    Thread mReaderThread;
    DataOutputStream sockOutput = null;

    private void logi(String msg) {
        String TAG = "LocationQESDK.NtripClientSocket";
        logger.logi(TAG, msg);
    }

    public void sendGGANmea(double latitude, double longitude, double altitude) {

        int utcHours, utcMinutes, utcSeconds, utcMSeconds;

        Instant instant = Instant.now();
        Date now = Date.from(instant);
        utcHours = now.getHours();
        utcMinutes = now.getMinutes();
        utcSeconds = now.getSeconds();
        utcMSeconds = (int)(instant.toEpochMilli() % 1000);

        int latFloor = (int)Math.floor(latitude);
        int longFloor = (int)Math.floor(longitude);

        char latHemisphere;
        char longHemisphere;

        if (latitude > 0) {
            latHemisphere = 'N';
        } else {
            latHemisphere = 'S';
            latitude *= -1.0;
        }

        if (longitude < 0) {
            longHemisphere = 'W';
            longitude *= -1.0;
        } else {
            longHemisphere = 'E';
        }

        double latMinutes = (latitude * 60.0) % 60.0;
        double longMinutes = (longitude * 60.0) % 60.0;

        String ggaContentFormat =
                "GNGGA,%02d%02d%02d.%02d,%02d%09.6lf,%c,%03d%09.6lf,%c,2,12,0.6,%.1lf,M,-9.0,M,36.2,0650";
        String ggaContent = String.format(ggaContentFormat,
                                utcHours, utcMinutes, utcSeconds, utcMSeconds/10,
                                latFloor, latMinutes, latHemisphere,
                                longFloor, longMinutes, longHemisphere, altitude);

        char checksum = 0;
        for (int idx = 0; idx < ggaContent.length(); idx++) {
            char c = ggaContent.charAt(idx);
            checksum ^= c;
        }

        String ggaNmeaFormat = "$%s*%02X\r\n";
        mGGANmea = String.format(ggaNmeaFormat, ggaContent, checksum);

        if (sockOutput != null) {
            try {
                sockOutput.writeBytes(mGGANmea);
            } catch (IOException e) {
                logi("Failed to write bytes over socket: " + e.getMessage());
            }
            logi("Sent NMEA GGA to NTRIP Server!");
        }
    }

    @SuppressLint("DefaultLocale")
    public NtripClientSocket(
            NtripClient.LogIface logIface, boolean useNtrip2Version, String hostName,
            String mountPoint, String userNamePwdEncodedInBase64Format, int port,
            NtripClient.NtripConfig config, NtripClient.CorrectionDataCallback corrDataCb) {

        logger = logIface;
        mUseNtrip2Version = useNtrip2Version;
        mHostName = hostName;
        mMountPoint = mountPoint;
        mUserNamePwdEncodedInBase64Format = Base64.getEncoder().encodeToString(userNamePwdEncodedInBase64Format.getBytes());
        mPort = port;
        mCorrectionDataCb = corrDataCb;

        mMaxContentDataBufferSizeInBytes = 2000;
        mTimeoutForFullBufferInSec = 0;
        mSocketReadRetryAttempts = 5;
        mWaitTimeBetweenReconnectAttemptsInSec = 5;
        mDataContentBuffer = null;
        mLogCorrectionData = false;
        mPacketCount = 0;
        mSocketFd = -1;
        mStopRequested = false;
        mReaderThread = null;

        if (config != null && config.maxContentDataBufferSizeInBytes != null) {
            mMaxContentDataBufferSizeInBytes = config.maxContentDataBufferSizeInBytes;
        }
        if (config != null && config.maxTimeoutForFullBufferInSec != null) {
            mTimeoutForFullBufferInSec = config.maxTimeoutForFullBufferInSec;
        }
        if (config != null && config.maxSocketReadRetryAttempts != null) {
            mSocketReadRetryAttempts = config.maxSocketReadRetryAttempts;
        }
        if (config != null && config.waitTimeBetweenReconnectAttemptsInSec != null) {
            mWaitTimeBetweenReconnectAttemptsInSec = config.waitTimeBetweenReconnectAttemptsInSec;
        }
        if (config != null && config.logCorrectionData != null) {
            mLogCorrectionData = config.logCorrectionData;
        }

        logi(String.format("HostName: %s, MountPoint: %s, " +
                        "UserNamePwdInBase64Format: %s, " +
                        "Port: %d, " +
                        "MaxContentDataBufferSizeInBytes: %d, " +
                        "MaxTimeoutForFullBufferInSec: %d, " +
                        "MaxSocketReadRetryAttempts : %d, " +
                        "WaitTimeBetweenReconnectAttemptsInSec: %d " +
                        "UseNtrip2Version: %b " +
                        "LogCorrectionData: %b",
                mHostName, mMountPoint, mUserNamePwdEncodedInBase64Format,
                mPort, mMaxContentDataBufferSizeInBytes,
                mTimeoutForFullBufferInSec, mSocketReadRetryAttempts,
                mWaitTimeBetweenReconnectAttemptsInSec, mUseNtrip2Version,
                mLogCorrectionData));
    }

    private long getBootTimeMilliSec() {
        return System.currentTimeMillis();
    }

    public int init() {
        mDataContentBuffer = new byte[mMaxContentDataBufferSizeInBytes];
        return 0;
    }

    public void startListening() {

        mStopRequested = false;
        mReaderThread = new Thread(this::startListeningAsync);
        mReaderThread.start();
    }

    public void stopListening() {

        mStopRequested = true;
        if (mReaderThread != null) {
            try {
                mReaderThread.join();
            } catch (InterruptedException e) {
                logi(e.getMessage());
            }
        }
    }

    @SuppressLint("DefaultLocale")
    private boolean dispatchAccumulatedBuffer(
            long timeWhenBufferingStartedMs, int totalAccumulatedBytes) {

        long nowMs = getBootTimeMilliSec();

        if (((nowMs - timeWhenBufferingStartedMs) >= (mTimeoutForFullBufferInSec * 1000L)) ||
                (totalAccumulatedBytes >= mMaxContentDataBufferSizeInBytes)) {

            if (mCorrectionDataCb != null) {

                byte[] injectData = new byte[totalAccumulatedBytes];

                System.arraycopy(mDataContentBuffer, 0, injectData, 0, totalAccumulatedBytes);

                mCorrectionDataCb.injectCorrectionData(injectData);

                logi(String.format("Send accumulated bytes : %d, elapsed time (ms): %d",
                        totalAccumulatedBytes,
                        nowMs - timeWhenBufferingStartedMs));

                if (mLogCorrectionData) {

                    logi(String.format("Packet: %d Size: %d",
                            mPacketCount, totalAccumulatedBytes));

                    StringBuilder builder = new StringBuilder();
                    builder.append("[ ");
                    for (byte b : mDataContentBuffer) {
                        builder.append(String.valueOf(b));
                    }
                    builder.append("]");
                    logi(builder.toString());
                }

                mPacketCount++;
            }

            return true;
        }

        return false;
    }

    @SuppressLint("DefaultLocale")
    void startListeningAsync() {

        int retryAttempts;
        int remainingBytes;
        int readAtIndex;
        int totalAccumulatedBytes;
        long timeWhenBufferingStartedMs = 0;
        String httpDelimiter = "\r\n\r\n";
        boolean needsReconnect = true;
        boolean discardHttpResponsePacket;

        InputStream sockInput = null;
        Socket socket;

        while (!mStopRequested) {

            do {

                InetAddress serverAddr;

                try {
                    serverAddr = InetAddress.getByName(mHostName);
                } catch (UnknownHostException e) {
                    logi("Invalid host name: " + mHostName);
                    break;
                }


                try {
                    socket = new Socket(serverAddr, mPort);
                } catch (IOException e) {
                    logi("Socket creation failed: " + e.getMessage());
                    break;
                }

                logi(String.format("Resolved hostname %s -> %s ",
                        mHostName, serverAddr.getHostAddress()));

                logi("Trying to connect to socket with addr: " + serverAddr.getHostAddress());

                try {
                    socket.setSoTimeout(1000);
                } catch (SocketException e) {
                    logi("Failed to set socket timeout: " + e.getMessage());
                }

                // todo - check if socket send timeout option is required

                try {
                    sockInput = socket.getInputStream();
                } catch (IOException e) {
                    logi("Failed to get socket reader: " + e.getMessage());
                    break;
                }

                try {
                    sockOutput = new DataOutputStream(socket.getOutputStream());
                } catch (IOException e) {
                    logi("Failed to get socket output stream: " + e.getMessage());
                    break;
                }

                String connRequest = "GET /" + mMountPoint + " HTTP/1.1\r\n";
                connRequest += "Host: " + mHostName + "\r\n";
                if (mUseNtrip2Version) {
                    connRequest += "Ntrip-Version: Ntrip/2.0\r\n";
                }
                connRequest += "User-Agent: NTRIP GNR/1.0.0 (Win32)\r\n";
                connRequest += "Authorization: Basic " + mUserNamePwdEncodedInBase64Format + "\r\n";
                connRequest += "Connection: close\r\n\r\n";

                logi("Sending Request: " + connRequest);

                try {
                    sockOutput.writeBytes(connRequest);
                } catch (IOException e) {
                    logi("Failed to write bytes over socket: " + e.getMessage());
                    break;
                }

                needsReconnect = false;

            } while (false);

            if (needsReconnect) {

                logi(String.format("Will  reattempt connection in %d sec ...",
                        mWaitTimeBetweenReconnectAttemptsInSec));
                try {
                    Thread.sleep(mWaitTimeBetweenReconnectAttemptsInSec * 1000L);
                } catch (InterruptedException e) {
                    logi("thread sleep interrupted: " + e.getMessage());
                }
                continue;
            }

            // restore the retryAttempts
            retryAttempts = mSocketReadRetryAttempts;
            discardHttpResponsePacket = true;
            remainingBytes = mMaxContentDataBufferSizeInBytes;
            readAtIndex = 0;
            totalAccumulatedBytes = 0;

            while (!mStopRequested) {

                if (readAtIndex == 0) {
                    timeWhenBufferingStartedMs = getBootTimeMilliSec();
                    logi("Buffering started at: " + timeWhenBufferingStartedMs);
                }

                int returnedBytes = 0;
                try {
                    returnedBytes = sockInput.read(mDataContentBuffer, readAtIndex, remainingBytes);
                } catch (IOException e) {
                    logi("Socket Read failed: " + e.getMessage());
                }

                if (returnedBytes > 0) {

                    logi("Received data of bytes: "+ returnedBytes + " ,readIndex: " + readAtIndex + " ,remainingBytes: " + remainingBytes);

                    // restore the retry attempts
                    retryAttempts = mSocketReadRetryAttempts;

                    if (discardHttpResponsePacket) {

                        String dataContent = ByteBuffer.wrap(mDataContentBuffer, readAtIndex, returnedBytes).toString();
                        logi("dataContent: " + dataContent);
                        if (dataContent.contains(httpDelimiter)) {
                            if (dataContent.indexOf(httpDelimiter) == dataContent.length() - 4) {
                                logi("Only HTTP Header received");
                            } else {
                                logi("Content received with the HTTP Header, not expected !!");
                            }
                        }



                        //Send GGA NMEA to server
                        if (sockOutput != null) {
                            try {
                                sockOutput.writeBytes(mGGANmea);
                            } catch (IOException e) {
                                logi("Failed to write bytes over socket: " + e.getMessage());
                                break;
                            }
                            logi("Sent NMEA GGA to NTRIP Server!");
                        }

                        discardHttpResponsePacket = false;
                        remainingBytes = mMaxContentDataBufferSizeInBytes;
                        readAtIndex = 0;
                        logi("discardHttpResponsePacket set to false");
                        continue;

                    } else {
                        logi("Start NMEA GGA to NTRIP Server!");
                        readAtIndex += returnedBytes;
                        remainingBytes -= returnedBytes;
                        totalAccumulatedBytes += returnedBytes;

                    }

                    if (dispatchAccumulatedBuffer(timeWhenBufferingStartedMs, totalAccumulatedBytes)) {
                        remainingBytes = mMaxContentDataBufferSizeInBytes;
                        readAtIndex = 0;
                        totalAccumulatedBytes = 0;
                        timeWhenBufferingStartedMs = 0;
                    }

                } else {

                    logi("Returned Bytes: " + returnedBytes);

                    if (totalAccumulatedBytes > 0) {
                        if (dispatchAccumulatedBuffer(
                                timeWhenBufferingStartedMs, totalAccumulatedBytes)) {

                            remainingBytes = mMaxContentDataBufferSizeInBytes;
                            readAtIndex = 0;
                            totalAccumulatedBytes = 0;
                            timeWhenBufferingStartedMs = 0;
                        }
                    }

                    retryAttempts--;
                    if (retryAttempts <= 0) {
                        needsReconnect = true;
                        break;
                    }
                }
            }
        }
    }
}
