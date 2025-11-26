/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_AUDIO_HW_AMRWBPLUS_DEC_H_
#define _QC2AUDIO_AUDIO_HW_AMRWBPLUS_DEC_H_

#include "QC2AudioHwAmrDec.h"

namespace qc2audio {

class BufferList;

class QC2AudioHwAmrWbPlusDec : public QC2AudioHwAmrDec{
 public:

    QC2AudioHwAmrWbPlusDec(
            const std::string& codecName,
            uint32_t sessionId,
            const std::string& instanceName,
            const std::string& mime,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener);

    virtual ~QC2AudioHwAmrWbPlusDec() = default;

 protected:
    QC2Status onStart() override;
    void onOutputsDone(std::shared_ptr<QC2AudioHwBuffer> outBuf) override;
};

};  // namespace qc2audio

#endif  // _QC2AUDIO_AUDIO_HW_AMRWBPLUS_DEC_H_
