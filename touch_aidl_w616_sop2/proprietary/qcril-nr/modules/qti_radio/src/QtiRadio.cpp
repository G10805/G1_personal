/******************************************************************************
#  Copyright (c) 2017, 2021 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "hidl_impl/2.4/qti_radio_service_2_4.h"
#include "hidl_impl/2.3/qti_radio_service_2_3.h"
#include "hidl_impl/2.2/qti_radio_service_2_2.h"
#include "hidl_impl/2.1/qti_radio_service_2_1.h"
#include "hidl_impl/2.0/qti_radio_service_2_0.h"
#include "hidl_impl/1.0/qti_radio_service_1_0.h"

#include <cstring>
#include "QtiRadio.h"
#include <framework/Log.h>

static load_module<QtiRadio> sQtiRadio;

QtiRadio* getQtiRadio() {
  return &(sQtiRadio.get_module());
}

/*
 * 1. Indicate your preference for looper.
 * 2. Subscribe to the list of messages via mMessageHandler.
 * 3. Follow RAII practice.
 */
QtiRadio::QtiRadio() {
  mName = "QtiRadio";

  using std::placeholders::_1;
  mMessageHandler = {
    HANDLER(QcrilInitMessage, QtiRadio::handleQcrilInit),
    HANDLER(Nas5gConnectionIndMessage, QtiRadio::handleNas5gConnectionIndMessage),
    HANDLER(Nas5gStatusIndMessage, QtiRadio::handleNas5gStatusIndMessage),
    HANDLER(NasEndcDcnrIndMessage, QtiRadio::handleNasEndcDcnrIndMessage),
    HANDLER(Nas5gSignalStrengthIndMessage, QtiRadio::handleNas5gSignalStrengthIndMessage),
    HANDLER(Nas5gConfigInfoIndMessage, QtiRadio::handleNas5gConfigInfoIndMessage),
    HANDLER(NasUpperLayerIndInfoIndMessage, QtiRadio::handleNasUpperLayerIndInfoIndMessage),
#ifndef QMI_RIL_UTF
    HANDLER(rildata::DataNrIconTypeIndMessage, QtiRadio::handleDataNrIconTypeIndMessage),
#endif
  };
}

/* Follow RAII.
 */
QtiRadio::~QtiRadio() {
}

/*
 * Module specific initialization that does not belong to RAII .
 */
void QtiRadio::init() {
  Module::init();
}

/*
 * List of individual private handlers for the subscribed messages.
 */
void QtiRadio::handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  Log::getInstance().d("[" + mName +
                       "]: get_instance_id = " + std::to_string(msg->get_instance_id()));

  if (!mQtiRadioService) {
    for (auto svcImpl : getHalServiceImplFactory<QtiRadioServiceBase>()) {
      bool result = svcImpl->registerService(msg->get_instance_id());
      if (result) {
        Log::getInstance().d("[" + mName + "]: Registered!");
        mQtiRadioService = svcImpl;
        break;
      }
    }
  }
}

void QtiRadio::handleNas5gConnectionIndMessage(std::shared_ptr<Nas5gConnectionIndMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mQtiRadioService) {
    mQtiRadioService->notifyOnNrBearerAllocationChange(msg->is5gBearerStatus());
  }
}

void QtiRadio::handleNas5gStatusIndMessage(std::shared_ptr<Nas5gStatusIndMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  if (mQtiRadioService) {
    mQtiRadioService->notifyOn5gStatusChange((five_g_status)msg->is5gEnabled());
  }
}

void QtiRadio::handleNasEndcDcnrIndMessage(std::shared_ptr<NasEndcDcnrIndMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  five_g_endc_dcnr endc_dcnr_info;
  endc_dcnr_info.endc_available = (int32_t)msg->isEndcAvailable();
  endc_dcnr_info.restrict_dcnr = (int32_t)msg->isDcnrRestricted();
  if (mQtiRadioService) {
    mQtiRadioService->notifyOnNrDcParamChange(endc_dcnr_info);
  }
}

void QtiRadio::handleNas5gSignalStrengthIndMessage(
    std::shared_ptr<Nas5gSignalStrengthIndMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  five_g_signal_strength signal_strength;
  signal_strength.rsrp = msg->getRsrp();
  signal_strength.snr = msg->getSnr();
  if (mQtiRadioService) {
    mQtiRadioService->notifyOnSignalStrengthChange(signal_strength);
  }
}

void QtiRadio::handleNas5gConfigInfoIndMessage(std::shared_ptr<Nas5gConfigInfoIndMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  if (mQtiRadioService) {
    if (msg->is5gSA()) {
      mQtiRadioService->notifyOn5gConfigInfoChange(FIVE_G_CONFIG_TYPE_SA);
    } else {
      mQtiRadioService->notifyOn5gConfigInfoChange(FIVE_G_CONFIG_TYPE_NSA);
    }
  }
}

void QtiRadio::handleNasUpperLayerIndInfoIndMessage(
    std::shared_ptr<NasUpperLayerIndInfoIndMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

  five_g_upper_layer_ind_info upli_info;
  upli_info.upper_layer_ind_info_status =
      (five_g_upper_layer_ind_status)msg->isUpperLayerIndInfoAvailable();
  upli_info.plmn_list_status = (five_g_plmn_info_list_status)msg->isPlmnsListInfoAvailable();
  if (mQtiRadioService) {
    mQtiRadioService->notifyOnUpperLayerIndInfoChange(upli_info);
  }
}

#ifndef QMI_RIL_UTF
void QtiRadio::handleDataNrIconTypeIndMessage(std::shared_ptr<rildata::DataNrIconTypeIndMessage> msg) {
  Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
  five_g_icon_type rilIconType;

  if (msg->isNone()) {
    rilIconType = FIVE_G_ICON_TYPE_NONE;
  } else if (msg->isBasic()) {
    rilIconType = FIVE_G_ICON_TYPE_BASIC;
  } else if (msg->isUwb()) {
    rilIconType = FIVE_G_ICON_TYPE_UWB;
  } else {
    rilIconType = FIVE_G_ICON_TYPE_INVALID;
  }

  if (mQtiRadioService) {
    mQtiRadioService->notifyOnNrIconTypeChange(rilIconType);
  }
}
#endif

#ifdef QMI_RIL_UTF
void qcril_qmi_qti_radio_service_init(int instanceId) {
  QCRIL_LOG_DEBUG("qcril_qmi_qti_radio_service_init %d", instanceId);
  auto msg = std::make_shared<QcrilInitMessage>((qcril_instance_id_e_type)instanceId);
  getQtiRadio()->dispatchSync(msg);
}
#endif
