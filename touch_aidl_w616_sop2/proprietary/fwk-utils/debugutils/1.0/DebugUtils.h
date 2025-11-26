/******************************************************************************
Copyright (c) 2020,2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#pragma once

#include <vendor/qti/hardware/debugutils/1.0/IDebugUtils.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor::qti::hardware::debugutils::implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct DebugUtils : public V1_0::IDebugUtils {
    // Methods from ::vendor::qti::hardware::debugutils::V1_0::IDebugUtils follow.
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> setBreakPoint(uint64_t pid, bool isProcess, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> setWatchPoint(uint64_t pid, uint64_t add, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> collectFullBinderDebugInfo(bool isBlocking, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> collectBinderDebugInfoByPid(uint64_t pid, bool isBlocking, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> collectBinderDebugInfoByProcessname(const hidl_string& processName, bool isBlocking, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> collectUserspaceLogs(const hidl_string& logCmd, const hidl_string& debugTag, uint64_t duration) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> collectStackTraceByProcessName(const hidl_string& processName, bool isJava, bool isBlocking, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> collectStackTraceByPid(uint64_t pid, bool isJava, bool isBlocking, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> startPerfettoTracing(uint64_t duration, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> stopPerfettoTracing(const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> startSimplePerfTracing(uint64_t pid, uint64_t duration, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> stopSimplePerfTracing(const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> executeDumpsysCommands(const hidl_string& command, bool isBlocking, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> collectDependentProcessStackTrace(uint64_t pid, bool isBlocking, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> collectRamdump(const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> collectMemoryInfo(bool isBlocking, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> collectCPUInfo(bool isBlocking, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> collectProcessMemoryInfo(bool isBlocking, uint64_t pid, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> collectProcessCPUInfo(bool isBlocking, uint64_t pid, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> collectPeriodicTraces(uint64_t pid, uint64_t duration, const hidl_string& debugTag) override;
    Return<::vendor::qti::hardware::debugutils::V1_0::Status> collectHprof(bool isBlocking, uint64_t pid, const hidl_string& debugTag) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};
 extern "C" V1_0::IDebugUtils* HIDL_FETCH_IDebugUtils(const char* name);

}  // namespace vendor::qti::hardware::debugutils::implementation
