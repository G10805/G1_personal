/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.data.cneEthernetService;

import android.content.pm.PackageManager;
import android.content.pm.PackageInfo;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;

import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.util.Log;

public class EthernetRequestHandler extends Service {

    public static final String LOGTAG = "cneEthernetService";

    private NetworkRequest cneEthernetMMSRequest;
    private NetworkRequest cneEthernetSUPLRequest;
    private NetworkRequest cneEthernetDUNRequest;
    private NetworkRequest cneEthernetFOTARequest;
    private NetworkRequest cneEthernetIMSRequest;
    private NetworkRequest cneEthernetCBSRequest;
    private NetworkRequest cneEthernetP2PRequest;
    private NetworkRequest cneEthernetIARequest;
    private NetworkRequest cneEthernetRCSRequest;
    private NetworkRequest cneEthernetXCAPRequest;
    private NetworkRequest cneEthernetEIMSRequest;

    private ConnectivityManager cm = null;

    private NetworkCallback mmsNetworkCallback = new NetworkCallback() {
        @Override
        public void onAvailable(Network network) {
            //No Op
        }

        @Override
        public void onLost(Network network) {
            // No op
        }
    };

    private NetworkCallback suplNetworkCallback = new NetworkCallback() {
      @Override
      public void onAvailable(Network network) {
          //No Op
      }

      @Override
      public void onLost(Network network) {
          // No op
      }
    };

    private NetworkCallback dunNetworkCallback = new NetworkCallback() {
      @Override
      public void onAvailable(Network network) {
          //No Op
      }

      @Override
      public void onLost(Network network) {
          // No op
      }
    };

    private NetworkCallback fotaNetworkCallback = new NetworkCallback() {
      @Override
      public void onAvailable(Network network) {
          //No Op
      }

      @Override
      public void onLost(Network network) {
          // No op
      }
    };

    private NetworkCallback imsNetworkCallback = new NetworkCallback() {
      @Override
      public void onAvailable(Network network) {
          //No Op
      }

      @Override
      public void onLost(Network network) {
          // No op
      }
    };

    private NetworkCallback cbsNetworkCallback = new NetworkCallback() {
      @Override
      public void onAvailable(Network network) {
          //No Op
      }

      @Override
      public void onLost(Network network) {
          // No op
      }
    };

    private NetworkCallback p2pNetworkCallback = new NetworkCallback() {
      @Override
      public void onAvailable(Network network) {
          //No Op
      }

      @Override
      public void onLost(Network network) {
          // No op
      }
    };

    private NetworkCallback iaNetworkCallback = new NetworkCallback() {
      @Override
      public void onAvailable(Network network) {
          //No Op
      }

      @Override
      public void onLost(Network network) {
          // No op
      }
    };

    private NetworkCallback rcsNetworkCallback = new NetworkCallback() {
      @Override
      public void onAvailable(Network network) {
          //No Op
      }

      @Override
      public void onLost(Network network) {
          // No op
      }
    };

    private NetworkCallback xcapNetworkCallback = new NetworkCallback() {
      @Override
      public void onAvailable(Network network) {
          //No Op
      }

      @Override
      public void onLost(Network network) {
          // No op
      }
    };

    private NetworkCallback eimsNetworkCallback = new NetworkCallback() {
      @Override
      public void onAvailable(Network network) {
          //No Op
      }

      @Override
      public void onLost(Network network) {
          // No op
      }
    };

    @Override
    public void onCreate() {
        super.onCreate();
        getPackageVersionInfo();

        /* SUPL / MMS / INTERNET / WIFI_P2P are non restricted capabilities. So placing
         * NRs with those capabilities. For other networks, we need
         * CONNECTIVITY_USE_RESTRICTED_NETWORKS permission.
         */
        cneEthernetMMSRequest = new NetworkRequest.Builder()
                .addCapability(NetworkCapabilities.NET_CAPABILITY_MMS)
                .addTransportType(NetworkCapabilities.TRANSPORT_ETHERNET)
                .build();
        cneEthernetSUPLRequest = new NetworkRequest.Builder()
                .addCapability(NetworkCapabilities.NET_CAPABILITY_SUPL)
                .addTransportType(NetworkCapabilities.TRANSPORT_ETHERNET)
                .build();
        cneEthernetDUNRequest = new NetworkRequest.Builder()
                .addCapability(NetworkCapabilities.NET_CAPABILITY_DUN)
                .addTransportType(NetworkCapabilities.TRANSPORT_ETHERNET)
                .build();
        cneEthernetFOTARequest = new NetworkRequest.Builder()
                .addCapability(NetworkCapabilities.NET_CAPABILITY_FOTA)
                .addTransportType(NetworkCapabilities.TRANSPORT_ETHERNET)
                .build();
        cneEthernetIMSRequest = new NetworkRequest.Builder()
                .addCapability(NetworkCapabilities.NET_CAPABILITY_IMS)
                .addTransportType(NetworkCapabilities.TRANSPORT_ETHERNET)
                .build();
        cneEthernetCBSRequest = new NetworkRequest.Builder()
                .addCapability(NetworkCapabilities.NET_CAPABILITY_CBS)
                .addTransportType(NetworkCapabilities.TRANSPORT_ETHERNET)
                .build();
        cneEthernetP2PRequest = new NetworkRequest.Builder()
                .addCapability(NetworkCapabilities.NET_CAPABILITY_WIFI_P2P)
                .addTransportType(NetworkCapabilities.TRANSPORT_ETHERNET)
                .build();
        cneEthernetIARequest = new NetworkRequest.Builder()
                .addCapability(NetworkCapabilities.NET_CAPABILITY_IA)
                .addTransportType(NetworkCapabilities.TRANSPORT_ETHERNET)
                .build();
        cneEthernetRCSRequest = new NetworkRequest.Builder()
                .addCapability(NetworkCapabilities.NET_CAPABILITY_RCS)
                .addTransportType(NetworkCapabilities.TRANSPORT_ETHERNET)
                .build();
        cneEthernetXCAPRequest = new NetworkRequest.Builder()
                .addCapability(NetworkCapabilities.NET_CAPABILITY_XCAP)
                .addTransportType(NetworkCapabilities.TRANSPORT_ETHERNET)
                .build();
        cneEthernetEIMSRequest = new NetworkRequest.Builder()
                .addCapability(NetworkCapabilities.NET_CAPABILITY_EIMS)
                .addTransportType(NetworkCapabilities.TRANSPORT_ETHERNET)
                .build();
        cm = (ConnectivityManager) this.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (cm != null) {
            try {
                cm.requestNetwork(cneEthernetMMSRequest, mmsNetworkCallback);
                cm.requestNetwork(cneEthernetSUPLRequest, suplNetworkCallback);
                cm.requestNetwork(cneEthernetDUNRequest, dunNetworkCallback);
                cm.requestNetwork(cneEthernetFOTARequest, fotaNetworkCallback);
                cm.requestNetwork(cneEthernetIMSRequest, imsNetworkCallback);
                cm.requestNetwork(cneEthernetCBSRequest, cbsNetworkCallback);
                cm.requestNetwork(cneEthernetP2PRequest, p2pNetworkCallback);
                cm.requestNetwork(cneEthernetIARequest, iaNetworkCallback);
                cm.requestNetwork(cneEthernetRCSRequest, rcsNetworkCallback);
                cm.requestNetwork(cneEthernetXCAPRequest, xcapNetworkCallback);
                cm.requestNetwork(cneEthernetEIMSRequest, eimsNetworkCallback);
            } catch (Exception e) {
                Log.e(LOGTAG, e.toString(), e);
            }
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void getPackageVersionInfo(){
        try {
            PackageInfo pInfo = this.getPackageManager().
                              getPackageInfo(this.getPackageName(),0);
            Log.i(LOGTAG, "Running with versionCode "
                    + pInfo.versionCode + " and versionName "+ pInfo.versionName );
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
    }
}
