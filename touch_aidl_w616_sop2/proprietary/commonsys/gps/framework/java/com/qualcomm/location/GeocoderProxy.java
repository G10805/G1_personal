/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.geocoder;

import android.location.Geocoder;
import android.location.Geocoder.GeocodeListener;
import android.location.Location;
import android.location.Address;
import android.content.Context;
import android.os.Message;
import android.os.Handler;
import android.util.Log;
import java.util.List;
import java.util.Locale;
import java.io.IOException;
import java.lang.IllegalArgumentException;
import android.os.RemoteException;

import vendor.qti.gnss.ILocAidlGnss;
import vendor.qti.gnss.ILocAidlGeocoderCallback;
import vendor.qti.gnss.ILocAidlGeocoder;
import vendor.qti.gnss.LocAidlLocation;
import vendor.qti.gnss.LocAidlAddress;

import com.qualcomm.location.idlclient.*;
import com.qualcomm.location.idlclient.LocIDLClientBase;
import com.qualcomm.location.utils.IZatServiceContext;

public class GeocoderProxy implements Handler.Callback {

    private static final String TAG = "GeocoderService";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);
    private static final int MSG_INIT = IZatServiceContext.MSG_GEOCODER_PROXY_BASE;
    private static final int MSG_REQUEST_ADDR = IZatServiceContext.MSG_GEOCODER_PROXY_BASE + 1;

    private static final Object mLock = new Object();
    private Handler mHandler;
    private Geocoder mGeocoder;
    private Geocoder.GeocodeListener mGeocodeListener;
    private IZatServiceContext mIZatServiceCtx;
    private final Context mCtx;
    private GeocoderIdlClient mIdlClient;
    private Location mLoc;

    private enum GeocoderStatus {
        UNKNOWN,
        PRESENT,
        NOT_PRESENT
    };

    private GeocoderStatus mGeocoderPresence = GeocoderStatus.UNKNOWN;

    public GeocoderProxy(Context ctx) {
        mCtx = ctx;
        mIZatServiceCtx = IZatServiceContext.getInstance(mCtx);
        mHandler = new Handler(mIZatServiceCtx.getLooper(), this);
        //Only English civic address is supported for now
        mGeocoder = new Geocoder(ctx, Locale.ENGLISH);
        mHandler.sendEmptyMessage(MSG_INIT);
        mGeocodeListener = new GeocodeListener() {
            public void onGeocode(List<Address> addrList) {
                if (addrList != null && addrList.size() != 0 && mIdlClient != null) {
                    Log.d(TAG, "return Address: " + addrList.get(0).toString());
                    Address addr = addrList.get(0);
                    if (addr.getCountryCode() == null &&
                            addr.getCountryName() != null) {
                        String countryCode =
                            getCountryCodeFromName(addr.getCountryName());
                        if (countryCode != null) {
                            addr.setCountryCode(countryCode);
                        }
                    }
                    mIdlClient.injectLocationAndAddr(mLoc, addr);
                }
            }

        };
    }

    @Override
    public boolean handleMessage(Message msg) {
        int msgID = msg.what;
        Log.d(TAG, "handleMessage what - " + msgID);

        switch (msgID) {
            case MSG_INIT:
            {
                Log.d(TAG, "GeocoderProxy init");
                mIdlClient = new GeocoderIdlClient(this);
                break;
            }
            case MSG_REQUEST_ADDR:
            {
                mLoc = (Location)msg.obj;
                mGeocoder.getFromLocation(mLoc.getLatitude(), mLoc.getLongitude(), 1,
                        mGeocodeListener);
                break;
            }
        }
        return true;
    }

    private String getCountryCodeFromName(String countryName) {
        String[] isoCountryCodes = Locale.getISOCountries();
        for (String code : isoCountryCodes) {
            Locale locale = new Locale("", code);
            if (countryName.contains(locale.getDisplayCountry(Locale.ENGLISH))) {
                Log.d(TAG, "country code: " + code + ",countryName ori: " + countryName
                        + ", countryName from Locale: " + locale.getDisplayCountry(Locale.ENGLISH));
                return code;
            }
        }
        return null;
    }

    private GeocoderStatus isGeocoderPresent() {
        if (mGeocoderPresence == GeocoderStatus.UNKNOWN) {
            mGeocoderPresence =
                    mGeocoder.isPresent() ? GeocoderStatus.PRESENT : GeocoderStatus.NOT_PRESENT;
        }
        return mGeocoderPresence;
    }

    private void getAddrFromGeoCoder(Location loc) {
        if (GeocoderStatus.PRESENT == isGeocoderPresent()) {
            mHandler.obtainMessage(MSG_REQUEST_ADDR, loc).sendToTarget();
        }
    }

    // ======================================================================
    // geocoder proxy Java AIDL client
    // ======================================================================
    static class GeocoderIdlClient extends LocIDLClientBase
            implements LocIDLClientBase.IServiceDeathCb {
        private final String TAG = "GeocoderIdlClient";
        private final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

        private ILocAidlGeocoder mGeocoderIface;
        private LocAidlGeocoderCallback mCallback;

        private GeocoderIdlClient(GeocoderProxy proxy) {
            getGeocoderIface();
            mCallback = new LocAidlGeocoderCallback(proxy);
            if (mGeocoderIface != null) {
                try {
                    mGeocoderIface.setCallback(mCallback);
                    registerServiceDiedCb(this);
                } catch (RemoteException e) {
                    Log.e(TAG, "exception happens when setCallback: " + e);
                }
            }
        }

        private void getGeocoderIface() {
            if (null == mGeocoderIface) {
                ILocAidlGnss service = (ILocAidlGnss)getGnssAidlService();

                if (null != service) {
                    try {
                        mGeocoderIface = service.getExtensionLocAidlGeocoder();
                    } catch (RemoteException e) {
                        Log.e(TAG, "Exception getting geocoder aidl iface: " + e);
                        mGeocoderIface = null;
                    }
                }
            }
        }

        @Override
        public void onServiceDied() {
            mGeocoderIface = null;
            getGeocoderIface();
            if (mGeocoderIface != null) {
                try {
                    mGeocoderIface.setCallback(mCallback);
                } catch (RemoteException e) {
                    Log.e(TAG, "exception happens when setCallback: " + e);
                }
            }
        }

        public void injectLocationAndAddr(Location loc, Address addr) {
            IDLClientUtils.toIDLService(TAG);
            LocAidlLocation aidlLoc = IDLClientUtils.convertAfwLocationToLocLocation(loc);
            LocAidlAddress aidlAddr = IDLClientUtils.convertAfwAddrToAidlAddr(addr);
            if (mGeocoderIface != null) {
                try {
                    mGeocoderIface.injectLocationAndAddr(aidlLoc, aidlAddr);
                } catch (RemoteException e) {
                    Log.e(TAG, "exception happens when injectLocationAndAddr: " + e);
                }
            }
        }

        // ======================================================================
        // Callbacks
        // ======================================================================
        class LocAidlGeocoderCallback extends ILocAidlGeocoderCallback.Stub {

            private GeocoderProxy mGeocoderProxy;

            private LocAidlGeocoderCallback(GeocoderProxy proxy) {
                mGeocoderProxy = proxy;
            }

            @Override
            public void getAddrFromLocationCb(LocAidlLocation loc) {
                IDLClientUtils.fromIDLService(TAG);
                Location afwLoc = IDLClientUtils.translateAidlLocation(loc);
                mGeocoderProxy.getAddrFromGeoCoder(afwLoc);
            }

            @Override
            public final int getInterfaceVersion() {
                return ILocAidlGeocoderCallback.VERSION;
            }
            @Override
            public final String getInterfaceHash() {
                return ILocAidlGeocoderCallback.HASH;
            }
        }
    }
}
