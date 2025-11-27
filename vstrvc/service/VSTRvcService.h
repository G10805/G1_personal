/**
*
*/
#ifndef __VISTEON_VST_RVCSERVICE_H
#define __VISTEON_VST_RVCSERVICE_H

#include <stdint.h>
#include <sys/types.h>

#include <cutils/compiler.h>
#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/SortedVector.h>
#include <utils/threads.h>
#include <map>

#include <binder/BinderService.h>

#include "IVSTRvcService.h"
#include "VST_rvc_impl.h"

extern void set_hmi_ready_ack();
extern void set_ivi_service_ready();
extern void set_hmi_timeout_ready();
extern void set_fastrvcexit();
extern void reset_hmi_ready();
extern void* monitor_rvc(void* data);
extern int power_mode;
extern int timeout_count;
extern int gpio_value_rvc;

namespace VSTRvc {

using namespace android;
using namespace std;

// ---------------------------------------------------------------------------
enum {

	RVC_STATE_RUNNING = 0,
	RVC_STATE_IDLE
};

class VSTRvcService : public BinderService<VSTRvcService>,
                       public BnVSTRvcService
{
public:
    static char const* getServiceName() {
        return "VSTRvcService";
    }

    VSTRvcService();
    ~VSTRvcService() {};

    virtual void setRvcBounds(const int32_t left, const int32_t top, const int32_t right, const int32_t bottom);
    virtual void startRvcCamera();
    virtual void stopRvcCamera();
    virtual bool hasStarted();
    virtual status_t dump(int fd, const Vector<String16>& args);
    void setAnimStatus(std::string state);
    void startBL();
    std::string getAnimState();
    void cleanUp();
    bool mBootAnimStopped;
private:
    display_parameters mDisplayParameters;
    int mRvcState;
};

// ---------------------------------------------------------------------------
} // namespace VSTRvc



#endif
