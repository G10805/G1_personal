/*====================================================================
*  Copyright (c) Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*====================================================================
*/

package vendor.qti.hardware.dsp;

import vendor.qti.hardware.dsp.IDspManager;

@VintfStability
interface IDspService {
    /**
     * API: getNewDSPSession
     * Description: this API acquires a DSPManager instance, which can be used to later open a DSP sessions.
     *              Synchronous.
     *
     * Input: flags        -> Session parameters
     *
     * Return: IDspManager -> Manager instance for further communication to fetch device fd
     */
    IDspManager getNewDSPSession(in int flags);
}
