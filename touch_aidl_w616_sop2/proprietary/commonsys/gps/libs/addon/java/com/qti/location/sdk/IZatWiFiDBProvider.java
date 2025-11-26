/* ======================================================================
*  Copyright (c) 2018, 2023 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/
package com.qti.location.sdk;

import android.location.Location;

import com.qti.location.sdk.IZatDBCommon.IZatCellInfo;
import com.qti.location.sdk.IZatDBCommon.IZatAPSSIDInfo;
import com.qti.location.sdk.IZatDBCommon.IZatRangingBandWidth;
import com.qti.location.sdk.IZatDBCommon.IZatApBsListStatus;
import com.qti.location.sdk.IZatIllegalArgumentException;

import java.util.List;

/*
 * <p>Copyright (c) 2017 Qualcomm Technologies, Inc.</p>
 * <p>All Rights Reserved.</p>
 * <p>Confidential and Proprietary - Qualcomm Technologies, Inc</p>
 * <br/>
 * <p><b>IZatWiFiDBReceiver</b> interface @version 1.0.0 </p>
 */

/** @addtogroup IZatWiFiDBProvider
@{ */

/** APIs for
* injecting a Wi-Fi AP location database to a Qualcomm Location framework.
*
*/
public abstract class IZatWiFiDBProvider {
   protected final IZatWiFiDBProviderResponseListener mResponseListener;

   /**
    * Constructor - IZatWiFiDBProvider.
    *
    * @param listener Listener to receive Wi-Fi DB receiver
    *         responses. This parameter cannot be NULL, otherwise
    *         a {@link IZatIllegalArgumentException} is
    *         thrown.
    * @throws IZatIllegalArgumentException The listener parameter is NULL.
    * @return
    * None.
    */
   protected IZatWiFiDBProvider(IZatWiFiDBProviderResponseListener listener)
                               throws IZatIllegalArgumentException {
       if(null == listener) {
           throw new IZatIllegalArgumentException("Unable to obtain IZatWiFiDBProvider instance");
       }
       mResponseListener = listener;
   }


   /**
    * Requests a list of access points.
    * <p>
    * This function enables Wi-Fi database providers to request a list of APs
    * that require location information.
    * </p>
    *
    * @return
    * None.
    */
   public abstract void requestAPObsLocData();

/** @} */ /* end_addtogroup IZatWiFiDBProvider */

/** @addtogroup IZatWiFiDBProvider
@{ */

   /**
    * Interface class IZatWiFiDBProviderResponseListener.
    *
    * This interface
    * receives responses from a Wi-Fi database receiver in a Qualcomm
    * Location framework.
    */
   public interface IZatWiFiDBProviderResponseListener {

       /**
        * Response to an AP list request.
        * <p>
        * This API is called by the underlying service back
        * to applications when a list of APs is available.
        * Applications should implement this interface.</p>
        *
        * @param ap_list List of APs.
        * @param ap_status Status of the AP list.
        * @param location Location of area of interest.
        *
        * @return
        * None.
        */
       void onApObsLocDataAvailable(List<IZatAPObsLocData> ap_obs_list,
                                    IZatApBsListStatus ap_obs_status);

       /**
        * Service request to a Wi-Fi database provider.
        * <p>
        * This API is called by the underlying service back
        * to applications when they need service. Applications should
        * implement this interface.</p>
        *
        * @return
        * None.
        */
       void onServiceRequest();
   }

/** @} */ /* end_addtogroup IZatWiFiDBProvider */

/** @addtogroup IZatWiFiDBProvider
@{ */

    /**
    * Class IZatAPObsLocData.
    */
    public static class IZatAPObsLocData {
        private Location mLocation;
        private IZatCellInfo mCellInfo;
        private long mScanTimestamp;
        private List<IZatAPScan> mApScanList;
        private List<IZatAPRttScan> mApRttScanList;

        /**
         * Constructor - IZatAPObsLocData for discovery scan
         *
         * @param location Location of the observation.
         * @param cellInfo Cell information of the observation.
         * @param apScanList List of AP scans.
         */
        public IZatAPObsLocData(Location location, IZatCellInfo cellInfo, long scanTimestamp,
                                List<IZatAPScan> apScanList) {
            this.mLocation = location;
            this.mCellInfo = cellInfo;
            this.mScanTimestamp = scanTimestamp;
            this.mApScanList = apScanList;
            this.mApRttScanList = null;
        }

        /**
         * Constructor - IZatAPObsLocData for ranging scan
         *
         * @param location Location of the observation.
         * @param timestamp timestamp of the observation.
         * @param apRttList List of AP Rtt scans.
         */
        public IZatAPObsLocData(List<IZatAPRttScan> apRttList, Location location,
                                IZatCellInfo cellInfo, long scanTimestamp) {
            this.mLocation = location;
            this.mCellInfo = cellInfo;
            this.mScanTimestamp = scanTimestamp;
            this.mApRttScanList = apRttList;
            this.mApScanList = null;
        }

        /**
         * Gets the location of this observation.
         *
         * @return Location of the observation.
         */
        public Location getLocation() {
            return mLocation;
        }

        /**
         * Gets the Cell information of the observation.
         *
         * @return Cell information of the observation.
         */
        public IZatCellInfo getCellInfo() {
            return mCellInfo;
        }

        /**
         * Gets the List of AP scans.
         * Only valid for discovery scan data.
         *
         * @return List of AP scans, null if this is rtt scan data.
         */
        public List<IZatAPScan> getApScanList() {
            return mApScanList;
        }

        /**
         * Gets the List of AP RTT scans.
         * Only valid for RTT scan data.
         *
         * @return List of AP RTT scans, null if this is discovery scan data.
         */
        public List<IZatAPRttScan> getApRttList() {
            return mApRttScanList;
        }

        /**
         * Gets the Scan timestamp, UTC seconds since UNIX epoch.
         *
         * @return Scan timestamp
         */
        public long getScanTimestamp() {
            return mScanTimestamp;
        }
    }

/** @} */ /* end_addtogroup IZatWiFiDBProvider */

/** @addtogroup IZatWiFiDBProvider
@{ */

    /**
    * Class IZatAPScan.
    */
    public static class IZatAPScan {
        private String mMacAddress;
        private float mRssi;
        private int mDeltatime;
        private IZatAPSSIDInfo mSSID;
        private int mAPChannelNumber;

        public enum IzatApServiceStatus {
            UNKNOWN,
            SERVING,
            NOT_SERVING,
        }

        private IzatApServiceStatus mIsServing;
        private int mFrequency;
        private IZatRangingBandWidth mBandWidth;

        /**
         * Deprecated
         * Constructor - IZatAPScan.
         *
         * @param macAddress The MAC address of the AP.
         * @param rssi The RSSI signal received in the scan.
         * @param deltatime Delta time since the scan started.
         * @param SSID SSID of the AP.
         * @param APChannelNumber Channel number of the AP.
         */
        public IZatAPScan(String macAddress, float rssi, int deltatime, IZatAPSSIDInfo SSID,
                          int APChannelNumber) {
           this.mMacAddress = macAddress;
           this.mRssi = rssi;
           this.mDeltatime = deltatime;
           this.mSSID = SSID;
           this.mAPChannelNumber = APChannelNumber;
        }

        /**
         * Constructor - IZatAPScan.
         *
         * @param macAddress The MAC address of the AP.
         * @param rssi The RSSI signal received in the scan.
         * @param deltatime Delta time since the scan started.
         * @param SSID SSID of the AP.
         * @param APChannelNumber Channel number of the AP.
         * @param isServing whether AP is serving/connected.
         * @param frequency Channel frequency in MHz.
         * @param bandWidth Frequency channel bandwidth in MHz.
         */
        public IZatAPScan(String macAddress, float rssi, int deltatime, IZatAPSSIDInfo SSID,
                          int APChannelNumber, IzatApServiceStatus isServing,
                          int frequency, IZatRangingBandWidth bandWidth) {
           this.mMacAddress = macAddress;
           this.mRssi = rssi;
           this.mDeltatime = deltatime;
           this.mSSID = SSID;
           this.mAPChannelNumber = APChannelNumber;
           this.mIsServing = isServing;
           this.mFrequency = frequency;
           this.mBandWidth = bandWidth;
        }

        /**
         * Gets the MAC address of the AP.
         *
         * @return The MAC address of the AP.
         */
        public String getMacAddress() {
           return mMacAddress;
        }

        /**
         * Gets the RSSI signal received in the scan.
         *
         * @return The RSSI signal received in the scan.
         */
        public float getRssi() {
           return mRssi;
        }

        /**
         * Gets delta time since the scan started, in microseconds.
         *
         * @return Delta time since the scan started.
         */
        public int getDeltatime() {
           return mDeltatime;
        }

        /**
         * Gets the SSID of the AP.
         *
         * @return SSID of the AP.
         */
        public IZatAPSSIDInfo getSSID() {
           return mSSID;
        }

        /**
         * Gets the channel number of the AP.
         *
         * @return Channel number of the AP.
         */
        public int getAPChannelNumber() {
           return mAPChannelNumber;
        }

        /**
         * Gets the serving/connection status of the AP.
         *
         * @return is this AP serving or not.
         */
        public IzatApServiceStatus getIsServing() {
           return mIsServing;
        }

        /**
         * Gets the channel frequency of the AP in MHz.
         *
         * @return Channel frequency of the AP.
         */
        public int getAPChannelFrequency() {
           return mFrequency;
        }

        /**
         * Gets the channel frequency bandwidth of the AP in MHz.
         *
         * @return bandwidth in MHz.
         */
        public IZatRangingBandWidth getAPBandWidth() {
           return mBandWidth;
        }
   }

/** @} */ /* end_addtogroup IZatWiFiDBProvider */

/** @addtogroup IZatRangingMeasurement
@{ */

    /**
     * Class IZatRangingMeasurement.
     */
    public static class IZatRangingMeasurement {
        int mDistanceInMM;
        float mRSSI;
        IZatRangingBandWidth mTxBandWidth;
        IZatRangingBandWidth mRxBandWidth;
        int mChainNumber;

        /**
         * Contructor - IZatRangingMeasurement.
         *
         * @param distanceInMM Distance In millimeters.
         * @param rssi The rssi signal in dBm.
         * @param txBandWidth Bandwidth in MHz of the transmitted ack from the device to Wifi Node
         * @param rxBandWidth Bandwidth in MHz of the received frame from the Wifi Node
         * @param chainNumber Number of chain (antenna)
         *
         * @return
         * None.
         */
        public IZatRangingMeasurement(int distanceInMM, float rssi,
                IZatRangingBandWidth txBandWidth,
                IZatRangingBandWidth rxBandWidth,
                int chainNumber) {
            this.mDistanceInMM = distanceInMM;
            this.mRSSI = rssi;
            this.mTxBandWidth = txBandWidth;
            this.mRxBandWidth = rxBandWidth;
            this.mChainNumber = chainNumber;
        }

        /**
         * Gets rtt distance in millimeters.
         *
         * @return distance in mm.
         */
        public int getDistanceInMM() {
           return mDistanceInMM;
        }

        /**
         * Gets rssi in dBm.
         *
         * @return rssi in dBm.
         */
        public float getRssi() {
           return mRSSI;
        }

        /**
         * Gets rx bandwidth of the AP in MHz.
         *
         * @return bandwidth in MHz.
         */
        public IZatRangingBandWidth getRxBandWidth() {
           return mTxBandWidth;
        }

        /**
         * Gets tx bandwidth of the AP in MHz.
         *
         * @return bandwidth in MHz.
         */
        public IZatRangingBandWidth getTxBandWidth() {
           return mTxBandWidth;
        }

        /**
         * Gets antenna chain number.
         *
         * @return chain number.
         */
        public int getChainNumber() {
           return mChainNumber;
        }
    }

/** @} */ /* end_addtogroup IZatRangingMeasurement */

/** @addtogroup IZatAPRttScan
@{ */

    /**
    * Class IZatAPRttScan.
    */
    public static class IZatAPRttScan {
        private String mMacAddress;
        private int mDeltatime;
        private int mNumAttempted;
        private List<IZatRangingMeasurement> mRangingMeasurements;

        /**
         * Constructor - IZatAPRttScan.
         *
         * @param macAddress The MAC address of the AP.
         * @param deltaTime delta time of when RTT measurement was taken.
         * @param numAttempted Number of attempted RTT measurements.
         * @param rangingMeasurements List of ranging measurements.
         */
        public IZatAPRttScan(String macAddress, int deltaTime, int numAttempted,
                             List<IZatRangingMeasurement> rangingMeasurements) {
           this.mMacAddress = macAddress;
           this.mDeltatime = deltaTime;
           this.mNumAttempted = numAttempted;
           this.mRangingMeasurements = rangingMeasurements;
        }

        /**
         * Gets the MAC address of the AP.
         *
         * @return The MAC address of the AP.
         */
        public String getMacAddress() {
           return mMacAddress;
        }

        /**
         * Gets delta time since the scan started, in microseconds.
         *
         * @return delta time since the scan started.
         */
        public long getDeltatime() {
           return mDeltatime;
        }

        /**
         * Gets the Number of attempted RTT measurements.
         *
         * @return Number of attempted RTT measurements.
         */
        public int getNumAttempted() {
           return mNumAttempted;
        }

        /**
         * Gets the List of AP ranging measurements.
         *
         * @return List of AP ranging measurements..
         */
        public List<IZatRangingMeasurement> getApRangingList() {
            return mRangingMeasurements;
        }

   }

/** @} */ /* end_addtogroup IZatAPRttScan */
}
/** @} */ /* end_addtogroup IZatWiFiDBProvider */
