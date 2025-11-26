/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package vendor.qti.hardware.alarm;

@VintfStability
interface IAlarm {
    /**
     * Cancel rtc alarm.
     *
     * @return 0 if the operation is successful
     */
    int cancelAlarm();

    /**
     * Get rtc alarm time in rtc register.
     *
     * @return rtc alarm time(seconds)
     */
    long getAlarm();

    /**
     * Get current rtc time in rtc register.
     *
     * @return current rtc time(seconds)
     */
    long getRtcTime();

    /**
     * Set rtc alarm to rtc register. Once an alarm is set to rtc
     * register, the previous alarm is overridden.
     *
     * @param time rtc alarm time(seconds)
     * @return 0 if the operation is successful
     */
    int setAlarm(in long time);
}
