/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#include "QC2AudioHw.h"
#include "QC2AudioHwElite.h"
#include "QC2AudioHwGecko.h"
//#include "utils/include/QC2StreamParser.h"
#include <cmath>

#undef LOG_TAG
#define LOG_TAG "QC2AudioHw"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

namespace qc2audio {

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioHwFactory
///////////////////////////////////////////////////////////////////////////////////////////////
QC2Status QC2AudioHwFactory::CreateHwInstance( std::unique_ptr<QC2AudioHw>& hw){

    AudioHwType currentHw = TYPE_GECKO;
    //TODO: Read prop to identify DSP for current chipset and update currentHw

    switch(currentHw) {
        case TYPE_ELITE:
            hw = std::make_unique<QC2AudioHwElite>();
            break;
        case TYPE_GECKO:
            hw = std::make_unique<QC2AudioHwGecko>();
            break;
        default:
            break;
    };

    if (hw == nullptr) {
        return QC2_NO_MEMORY;
    } else {
        return QC2_OK;
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioHw
///////////////////////////////////////////////////////////////////////////////////////////////


};  //  namespace qc2audio
