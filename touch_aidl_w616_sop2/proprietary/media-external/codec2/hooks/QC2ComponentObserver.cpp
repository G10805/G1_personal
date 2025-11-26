/*
 **************************************************************************************************
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#include "QC2ComponentObserver.h"
#include "QC2PerfController.h"

namespace qc2 {

// default implementation of component-observer
class QC2DefaultCompObserver : public QC2ComponentObserver {
 public:
    virtual ~QC2DefaultCompObserver();

    QC2DefaultCompObserver(
        const std::string& componentName,
        ComponentKind kind,
        uint32_t id);

    void onStarted() override {
    }

    void onStopped() override {
    }

    void onTripped() override {
    }

    void onError(QC2Status errorCode, const char *errorMsg) override {
        (void)errorCode;
        (void)errorMsg;
    }

 private:
    std::unique_ptr<QC2PerfController> mPerfControl;
    bool isPerControlEnable;
};

QC2ComponentObserver::~QC2ComponentObserver() {
}

QC2DefaultCompObserver::QC2DefaultCompObserver(
        const std::string& componentName,
        ComponentKind kind,
        uint32_t id) {
    (void)id;
    isPerControlEnable = false;
    // isPerControlEnable = TargetSpec::Get()->isFeatureEnabled(
    //                                           "perf_control_enable");
    if (isPerControlEnable &&
        (kind == KIND_DECODER || kind == KIND_ENCODER)) {
        mPerfControl = std::make_unique<QC2PerfController>(componentName, kind);
        if (mPerfControl != nullptr) {
            mPerfControl->acquire();
        }
    }
}

QC2DefaultCompObserver::~QC2DefaultCompObserver() {
    if (mPerfControl != nullptr) {
        mPerfControl->release();
    }
}

// static
QC2Status QC2ComponentObserver::Create(
        const std::string& componentName,
        ComponentKind kind,
        uint32_t id,
        std::unique_ptr<QC2ComponentObserver> *observer) {
    if (observer == nullptr) {
        return QC2_ERROR;
    }
    *observer = std::unique_ptr<QC2ComponentObserver>(
            new QC2DefaultCompObserver(componentName, kind, id));
    return QC2_OK;
}

}  // namespace qc2
