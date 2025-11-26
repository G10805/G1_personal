/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.gnss;

/**
 * GNSS constellation type
 *
 * This is to specify the navigation satellite system, for example, as listed in Section 3.5 in
 * RINEX Version 3.04.
 */
@VintfStability
@Backing(type="byte")
enum LocAidlGnssConstellationType {
    UNKNOWN = 0,
    /**
     * Global Positioning System.
     */
    GPS = 1,
    /**
     * Satellite-Based Augmentation System.
     */
    SBAS = 2,
    /**
     * Global Navigation Satellite System.
     */
    GLONASS = 3,
    /**
     * Quasi-Zenith Satellite System.
     */
    QZSS = 4,
    /**
     * BeiDou Navigation Satellite System.
     */
    BEIDOU = 5,
    /**
     * Galileo Navigation Satellite System.
     */
    GALILEO = 6,
    /**
     * Indian Regional Navigation Satellite System.
     */
    IRNSS = 7,
}
