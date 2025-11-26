/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

// FIXME: license file, or use the -l option to generate the files with the header.

package vendor.qti.hardware.sensorscalibrate;

@VintfStability
interface ISensorsCalibrate {
    // Adding return type to method instead of out param String ret since there is only one return value.
    /**
     * @param  sensor_type: sensor type
     * @param  test_type: 0:sw test 1: hw test 2: factory test 3:COM test
     * @param out ret: a string including sensor name and test result
     *
     */
    String SensorsCal(in int sensor_type, in byte test_type);
}
