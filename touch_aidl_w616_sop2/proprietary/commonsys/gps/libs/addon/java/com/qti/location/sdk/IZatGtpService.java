/* ======================================================================
 *  Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 *  ====================================================================*/
package com.qti.location.sdk;

import android.location.Location;

/**
 * The IZatGtpService provides an interface for directly interacting with Qualcomm's network
 * location provider and receiving WWAN/WLAN-based terrestrial positioning services. Applications
 * need to get End User's consent to start using this service. Please note that it is an
 * application developer responsibility to comply with all relevant privacy and regulatory
 * requirements. Use of the API is subject to, and only authorized for parties who have executed,
 * a license for this feature.
 *
 * <pre>
 * <code>
 *
 *  // Sample Code
 *
 *  import android.app.Activity;
 *  import android.os.Bundle;
 *  import android.view.View;
 *  import android.widget.Button;
 *  import com.qti.location.sdk.IZatManager;
 *  import com.qti.location.sdk.IZatGtpService;
 *
 *  public class FullscreenActivity extends Activity {
 *
 *     private IZatManager mIzatMgr = null;
 *     private IZatGtpService mGtpService = null;
 *
 *     {@code @Override}
 *     protected void onCreate(Bundle savedInstanceState) {
 *          ...
 *          // get the instance of IZatManager
 *          mIzatMgr = IZatManager.getInstance(getApplicationContext());
 *          ...
 *          // create a callback object used to receive the location objects.
 *          final LocationCallback locationCb = new LocationCallback();
 *          ...
 *          final Button connectButton = (Button)findViewById(R.id.connectButton);
 *          connectButton.setOnClickListener(new View.OnClickListener() {
 *              {@code @Override}
 *              public void onClick(View v) {
 *                  // connect to IZatGtpService through IZatManager
 *                  if (mIzatMgr != null) {
 *                      mGtpService = mIzatMgr.connectToGtpService();
 *                  }
 *              }
 *          });
 *          ...
 *          // Capture user's consent to use GTP services
 *          final Button userConsentButton = (Button)findViewById(R.id.userConsentButton);
 *          connectButton.setOnClickListener(new View.OnClickListener() {
 *              {@code @Override}
 *              public void onClick(View v) {
 *                  mGtpService.setUserConsent(true);
 *              }
 *          });
 *          ...
 *          // Start Active session with specified interval
 *          final Button startButton = (Button)findViewById(R.id.startButton);
 *          startButton.setOnClickListener(new View.OnClickListener() {
 *              {@code @Override}
 *              public void onClick(View v) {
 *                  // start an active GTP session with specified time between fixes
 *                  if (mGtpService != null) {
 *                      int tbf = 1000; // time between fixes in milliseconds
 *                      IzatGtpAccuracy accuracy = HIGH;
 *                      mGtpService.requestLocationUpdates(locationCb, tbf, accuracy);
 *                  }
 *              }
 *          });
 *      }
 *  }
 *  public class LocationCallback implements IZatGtpService.IZatGtpServiceCallback {
 *      public void onLocationAvailable(Location loc) {
 *          ...
 *      }
 *  }
 * </code>
 * </pre>
 */
public interface IZatGtpService {

        /**
         * Enum IzatGtpAccuracy.
         *
         * IzatGtpAccuracy specified Gtp Accuracy, NOMINAL or HIGH
         */

        public enum IzatGtpAccuracy {

            NOMINAL(0),
            HIGH(1);

            private final int value;
            private IzatGtpAccuracy(int value) {
                this.value = value;
            }
            public int getValue() {
                return value;
            }
        }
    /**
     * Starts an active GTP session with specified time between fixes. If a session is already
     * active, it will be replaced with the minIntervalMillis provided
     *
     * @param callback Callback to receive locations. This parameter cannot be NULL, otherwise a
     * {@link IZatIllegalArgumentException} is thrown.
     * @param minIntervalMillis The desired time interval, in milliseconds, between location
     *                          fixes. Minimum accepted value is 1000. If a value less than 1000
     *                          is provided, 1000 will be used instead.
     * @throws IZatIllegalArgumentException One or more parameters are NULL.
     */
    void requestLocationUpdates(IZatGtpServiceCallback callback, int minIntervalMillis)
            throws IZatIllegalArgumentException;
    /**
     * Starts an active GTP session with specified time between fixes. If a session is already
     * active, it will be replaced with the minIntervalMillis provided
     *
     * @param callback Callback to receive locations. This parameter cannot be NULL, otherwise a
     * {@link IZatIllegalArgumentException} is thrown.
     * @param minIntervalMillis The desired time interval, in milliseconds, between location
     *                          fixes. Minimum accepted value is 1000. If a value less than 1000
     *                          is provided, 1000 will be used instead.
     * @param accuracy, gtp accuracy which includes HIGH and NOMINAL (default)
     * @throws IZatIllegalArgumentException One or more parameters are NULL.
     */
    void requestLocationUpdates(IZatGtpServiceCallback callback, int minIntervalMillis,
            IzatGtpAccuracy accuracy) throws IZatIllegalArgumentException;

    /**
     * Stops the active GTP session, if it exists
     */
    void removeLocationUpdates();

    /**
     * Register a callback for passive location updates.  A passive client gets location updates
     * via callback only when there are updates for other active clients.
     * @param callback Callback to receive locations.
     *         This parameter cannot be NULL, otherwise a
     *         {@link IZatIllegalArgumentException} is thrown.
     * @throws IZatIllegalArgumentException One or more parameters are NULL.
     */
    void requestPassiveLocationUpdates(IZatGtpServiceCallback callback)
            throws IZatIllegalArgumentException;

    /**
     * Removes registered callback, if any, from passive location updates
     */
    void removePassiveLocationUpdates();

    /**
     * This field Indicates whether Application Developer has obtained End User Opt-In to enable
     * this feature (must be set to true to enable positioning functionality). IzatGtpService
     * will keep the current user consent status persistent across device boot cycles, and
     * continue to use it until the application is uninstalled. Please note that it is an
     * application developer responsibility to comply with all relevant privacy and regulatory
     * requirements. Use of the API is subject to, and only authorized for parties who have
     * executed, a license for this feature.
     */
    void setUserConsent(boolean userConsent);

    /**
     * IGtpLocationCallback is the interface for receiving locations.
     *
     * <pre>
     * <code>
     *
     *  // Sample Code
     *
     *  import android.app.Activity;
     *  import android.os.Bundle;
     *  import android.view.View;
     *  import android.widget.Button;
     *  import android.util.Log;
     *  import com.qti.location.sdk.IZatManager;
     *  import com.qti.location.sdk.IZatGtpService;
     *
     *  public class FullscreenActivity extends Activity {
     *      ...
     *      // create a callback object used to receive the location objects.
     *      final LocationCallback locationCb = new LocationCallback();
     *      final int tbf = 1000;
     *
     *      ...
     *      final Button startButton = (Button)findViewById(R.id.startButton);
     *      startButton.setOnClickListener(new View.OnClickListener() {
     *          {@code @Override}
     *          public void onClick(View v) {
     *             ...
     *             // start the session.
     *             mHandle = mGtpService.requestLocationUpdates(locationCb, tbf);
     *          }
     *      });
     *  }
     *  public class LocationCallback implements IZatGtpService.IZatGtpServiceCallback {
     *      public void onLocationAvailable(Location location) {
     *          Log.d("IzatSDK", "Position measurement: " + location.toString());
     *      }
     *  }
     * </code>
     * </pre>
     */
    interface IZatGtpServiceCallback {
        /**
         * This API is called by the underlying service back
         * to applications when locations are available.
         * Applications should implement this interface.
         *
         * @param location Reported location.
         */
        void onLocationAvailable(Location location);
    }
}
