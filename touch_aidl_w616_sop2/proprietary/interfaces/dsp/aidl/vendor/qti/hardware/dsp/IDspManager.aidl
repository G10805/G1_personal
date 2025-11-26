/*====================================================================
*  Copyright (c) Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*====================================================================
*/

package vendor.qti.hardware.dsp;

import vendor.qti.hardware.dsp.DSPDomain;
import vendor.qti.hardware.dsp.DSPError;
import vendor.qti.hardware.dsp.FastrpcDev;

@VintfStability
interface IDspManager {
    /**
     * API: closeSession
     * Description: this API is used for closing an already opened device node
     *              with device fd for a given DSPDomain. Synchronous.
     *
     * Input: DSPDomain     -> structure consisting information of
     *                          remote domain id for opening device node
     *
     * Return: DSPError     -> Status indicating whether closing device node
     *                         is successful for the given DSPDomain
     */
    DSPError closeSession(in DSPDomain domain);


    /**
     * API: openSession
     * Description: this API is used for opening a device node
     *              with device fd for a given DSPDomain. Synchronous.
     *
     * Input: DSPDomain     -> structure consisting information of
     *                          remote domain id for opening device node
     *
     * Return: FastrpcDev   -> structure consisting of device fd and
     *                         error code, if any.
     */
    FastrpcDev openSession(in DSPDomain domain);
}
