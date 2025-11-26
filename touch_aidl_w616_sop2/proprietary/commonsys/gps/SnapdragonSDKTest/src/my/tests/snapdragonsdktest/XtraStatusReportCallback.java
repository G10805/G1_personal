/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package my.tests.snapdragonsdktest;

import com.qti.location.sdk.IZatDebugReportingService;
import com.qti.location.sdk.IZatDebugReportingService.IZatXtraStatus;
import com.qti.debugreport.*;

import android.util.Log;
import java.util.Date;
import java.text.SimpleDateFormat;
import java.io.File;
import java.io.PrintWriter;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

import android.os.Environment;
import java.util.Calendar;
import android.text.format.DateFormat;
import android.content.Context;

public class XtraStatusReportCallback implements IZatDebugReportingService.IZatXtraStatusCallback {
    private static String TAG = "XtraStatusCallbackInApp";

    private boolean mExternalStorageAvailable = false;
    private boolean mExternalStorageWritable = false;
    File mFileReport;
    FileOutputStream mFileOutputStream;
    PrintWriter mFileWriter;
    private static DebugReportActivity mDebugReportActivityObj;
    private static Context mAppContext;
    private int mReportNum;

    public XtraStatusReportCallback(DebugReportActivity objDebugReportActivity, Context appContext) {
        mDebugReportActivityObj = objDebugReportActivity;
        mAppContext = appContext;
        mReportNum = 0;

        checkExternalMedia();
        if (mExternalStorageWritable) {
            String xtraStatusReportFileFolderPath = mAppContext.getExternalFilesDir(null)
                .getAbsolutePath() + "/XtraStatusReport";
            Log.i(TAG, "root dir:" + xtraStatusReportFileFolderPath);
            File rootDir = new File(xtraStatusReportFileFolderPath);
            try {
                if (!rootDir.exists()) {
                    if (!rootDir.mkdirs()) {
                        throw(new Exception("unable to create folder" + xtraStatusReportFileFolderPath));
                    }
                }
                String timestamp = DateFormat.format("yyyy_MMM_dd_kkmm",
                    Calendar.getInstance()).toString();
                String logFileNameFormat = "%s/%s_%s.log";
                String logFileNameFormatRep = "%s/%s_%s_%d.log";
                final String prefix = "statusReport";
                String fileName = String.format(logFileNameFormat,
                    xtraStatusReportFileFolderPath, prefix, timestamp);
                mFileReport = new File(fileName);
                int i = 1;
                while (mFileReport.exists())
                {
                    fileName = String.format(logFileNameFormatRep, xtraStatusReportFileFolderPath,
                        prefix, timestamp,i);
                    mFileReport = new File(fileName);
                    i++;
                }
                try {
                    Log.i(TAG, "using file:" + fileName);
                    mFileOutputStream = new FileOutputStream(mFileReport);
                } catch (FileNotFoundException e) {
                    Log.e(TAG, e.getMessage());
                    e.printStackTrace();
                }
                mFileWriter = new PrintWriter(mFileOutputStream, true);
            } catch (Exception e) {
                Log.e(TAG, e.getMessage());
                e.printStackTrace();
            }
        }
    }

    public void onXtraStatusChanged(IZatXtraStatus xtraStatus) {
        Log.v(TAG, "Got xtraStatus in app");
        if (mFileWriter != null) {
            SimpleDateFormat s = new SimpleDateFormat("hh:mm:ss.SSS");
            String sDate = s.format(new Date());
            String sReportNum = "---- xtra status report :" + mReportNum;
            mFileWriter.println();
            mFileWriter.println(sDate + ":" + sReportNum);
            mReportNum ++;
        }
        mDebugReportActivityObj.printXtraStatusReport(xtraStatus, mFileWriter);
    }

    public void checkExternalMedia() {
         // check if external media is available
        String state = Environment.getExternalStorageState();

        if (Environment.MEDIA_MOUNTED.equals(state)) {
            // Can read and write the media
            mExternalStorageAvailable = mExternalStorageWritable = true;
        } else if (Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
            // Can only read the media
            mExternalStorageAvailable = true;
            mExternalStorageWritable = false;
        } else {
            // Can't read or write
            mExternalStorageAvailable = mExternalStorageWritable = false;
        }
        Log.d(TAG, "\n\nExternal Media: readable="
                +mExternalStorageAvailable+" writable="+mExternalStorageWritable);
    }

    public void saveReport() {
        mFileWriter.flush();
        mFileWriter.close();

        try {
            mFileOutputStream.close();
        } catch (IOException e) {
            Log.e(TAG, "IOException when closing FileOutput stream");
            e.printStackTrace();
        }

        mFileWriter = null;
        mFileOutputStream = null;
        mFileReport = null;
    }
}
