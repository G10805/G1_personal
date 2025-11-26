/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
package com.qualcomm.qti.locationqesdkdemo.tp1.ntrip;

public class NtripClient {

    private final String TAG = "LocationQESDK.NtripClient";
    private LogIface logger;
    private NtripClientSocket clientSocket;

    private void logi(String msg) {

        logger.logi(TAG, msg);
    }

    public NtripClient(LogIface logIface) {

        logger = logIface;
    }

    public void startCorrectionDataStreaming(
            boolean useNtrip2Version, String hostName, String mountPoint,
            String userNamePwdEncodedInBase64Format, int port, NtripConfig config,
            CorrectionDataCallback corrDataCb) {

        logi("Start streaming.");

        if (hostName.isEmpty() || mountPoint.isEmpty() ||
                userNamePwdEncodedInBase64Format.isEmpty()) {
            logi("Mandatory parameters are not provided");
            return;
        }

        if (clientSocket == null) {

            clientSocket = new NtripClientSocket(logger, useNtrip2Version, hostName,
                    mountPoint, userNamePwdEncodedInBase64Format, port, config,
                    corrDataCb);

            int retVal = clientSocket.init();
            if (retVal == 0) {
                clientSocket.startListening();
            } else {
                clientSocket = null;
            }
        } else {
            logi("Streaming in progress already !!");
        }
    }

    /** @brief API to stop streaming for correction data.
     */
    public void stopCorrectionDataStreaming() {

        logi("Stop streaming");

        if (clientSocket != null) {

            clientSocket.stopListening();
            clientSocket = null;
        }
    }

    public void sendGGANmea(double latitude, double longitude, double altitude) {
        if (clientSocket != null) {
            clientSocket.sendGGANmea(latitude, longitude, altitude);
        }
    }

    public class NtripConfig {
        // Max content data buffer size in bytes for NTRIP service
        Integer maxContentDataBufferSizeInBytes;
        // Max timeout for full buffer in sec for NTRIP service
        Integer maxTimeoutForFullBufferInSec;
        // Max Socket read retry attemps for NTRIP service
        Integer maxSocketReadRetryAttempts;
        // Wait time between reconnect attemps for NTRIP service
        Integer waitTimeBetweenReconnectAttemptsInSec;
        // Log correction data
        Boolean logCorrectionData;
    };

    public interface CorrectionDataCallback {

        void injectCorrectionData(byte[] buffer);
    }

    public interface LogIface {

        void logi(String tag, String msg);
    }
}
