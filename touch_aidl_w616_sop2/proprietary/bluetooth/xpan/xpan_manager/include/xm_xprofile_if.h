/*
 *  Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved..
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#pragma once

#ifndef XM_XPROFILE_IF_H
#define XM_XPROFILE_IF_H

#pragma once

#include <stdint.h>
#include "xpan_utils.h"
#include "xpan_manager_main.h"
#include "xm_ipc_if.h"
#include "xpan_provider_if.h"

namespace xpan {
namespace implementation {
using namespace std;
class XMXprofileIf
{
  public:
    XMXprofileIf();
    ~XMXprofileIf();
    void ProcessMessage(XmIpcEventId, xm_ipc_msg_t *);
    bool XmXpBearerSwitchInd(bdaddr_t, uint8_t);
    void Initialize(void);
    void Deinitialize(void);

  private:
    static void TransportEnabled(bdaddr_t, TransportType, bool, uint8_t);
    bool RemoteSupportsXpan(bdaddr_t, bool);
    bool UseCaseUpdate(UseCaseType);
    bool XmXpPrepareAudioBearerReq(bdaddr_t, TransportType);
    static void XpXmPrepareAudioBearerRsp(bdaddr_t, uint8_t, uint8_t);
    static void XpXmBearerSwitchInd(bdaddr_t, uint8_t, uint8_t);
    bool WiFiAcsResults(xm_ipc_msg_t *);
    static void HostParameters(macaddr_t, uint16_t);
    static void UpdateTWTSessionParams(uint8_t,
		              std::vector<tXPAN_Twt_Session_Params>);
    static void XpBearerPreferenceInd(uint8_t);
    bool TransportUpdate(TransportType);
    bool WifiTwtEvent(xm_ipc_msg_t *);
    static void UpdateXpanBondedDevices(uint8_t, bdaddr_t *);
    static void XpSapPowerSave(uint8_t, uint8_t);
    bool WifiSapPowerSaveEvent(xm_ipc_msg_t *);
    static void SapState(uint16_t);
    static void CreateSapInterface(uint8_t);
    static void EnableAcs(std::vector<uint32_t>);
    bool UpdateSapInterface(xm_ipc_msg_t *);
};

} // namespace implementation
} // namespace xpan

#endif
