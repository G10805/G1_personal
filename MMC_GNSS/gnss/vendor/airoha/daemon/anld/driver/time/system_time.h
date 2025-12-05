#pragma once

#include <stdint.h>

namespace airoha {
int64_t getSystemRealTimeInMs();
int64_t getSystemRealTimeInNs();
int64_t getSystemMonoTimeMs();
int64_t getSystemMonoTimeNs();
}
