#include <mutex>
#include <fstream>
#include <thread>
#include <cmath>
#include <iomanip>
#include <cutils/properties.h>
#include <time.h>

#include <android/hardware/automotive/vehicle/2.2/IVehicle.h>
#include <android/hardware/automotive/vehicle/2.0/IVehicleCallback.h>
#include <android/hardware/automotive/vehicle/2.2/types.h>
#include <vendor/visteon/system/vsthostconfiguration/1.0/types.h>
#include <vendor/visteon/system/vsthostconfiguration/1.0/IVstConfigInterface.h>

#include <log/log.h>
#include <string>

using android::hardware::automotive::vehicle::V2_2::IVehicle;
using android::hardware::automotive::vehicle::V2_2::VehicleProperty;
using android::hardware::automotive::vehicle::V2_2::VehiclePowerStateReport;

using android::hardware::automotive::vehicle::V2_0::IVehicleCallback;
using android::hardware::automotive::vehicle::V2_0::VehiclePropertyStatus;
using android::hardware::automotive::vehicle::V2_0::VehiclePropValue;
using android::hardware::automotive::vehicle::V2_0::StatusCode;
using android::hardware::automotive::vehicle::V2_0::SubscribeFlags;
using android::hardware::automotive::vehicle::V2_0::SubscribeOptions;
using android::hardware::automotive::vehicle::V2_0::VehiclePropConfig;
using vendor::visteon::system::vsthostconfiguration::V1_0::ConfigProperty;
using vendor::visteon::system::vsthostconfiguration::V1_0::IVstConfigInterface;
//#include "varconfig.h"
extern "C" int varconfig_main(); 
std::string varient;
std::mutex mAccessLock;
VehiclePropValue finalPropVal;
//varconfig* mConfigService = nullptr;
void getVehiclePropCallback(StatusCode status, const VehiclePropValue& propValue);
void getVehicleProperty(VehiclePropValue vHalPropInVal, VehiclePropValue& vHalPropOutVal);
VehiclePropValue reqPropVal = {},resultPropVal = {};
android::sp<IVstConfigInterface> mConfigService=vendor::visteon::system::vsthostconfiguration::V1_0::IVstConfigInterface::getService();
std::function <void (StatusCode, const VehiclePropValue&)> fvhalCallback = getVehiclePropCallback;
static int gnss_varient;

/*varconfig *varconfig::OBJvarient = NULL;

varconfig::varconfig()
{
}

varconfig::varconfig(const varconfig &)
{
}

varconfig &varconfig::operator=(const varconfig &)
{
  varconfig *pvarconfig = NULL;
  return *pvarconfig;
}

varconfig::~varconfig()
{
  OBJvarient = NULL;
}

varconfig *varconfig::getInstance()
{
  if (NULL == OBJvarient)
  {
    OBJvarient = new (std::nothrow) varconfig;
  }
  return OBJvarient;
}*/
void getVehiclePropCallback(StatusCode status, const VehiclePropValue& propValue) {
    if(status == StatusCode::NOT_AVAILABLE){
        ALOGI("getVehiclePropCallback::status not available");

    } else if( status == StatusCode::OK){
        ALOGV("getVehiclePropCallback::propId = 0x%x start", propValue.prop);
    }
    finalPropVal = propValue;
    return;
}
void getVehicleProperty(VehiclePropValue vHalPropInVal, VehiclePropValue& vHalPropOutVal) {

 
 std::lock_guard<std::mutex> lock(mAccessLock);
    if (mConfigService == nullptr)
    {
        ALOGW("getVehicleProperty()::mConfigService is null");
    }
    else
    {
        ALOGV("getVehicleProperty::propId = 0x%x start",vHalPropInVal.prop);
        memset(&finalPropVal,0,sizeof(finalPropVal));
		
		mConfigService->get(vHalPropInVal, fvhalCallback);      

    vHalPropOutVal = finalPropVal;
    ALOGW("getVehicleProperty end");

    }
}
int get_varient()
   {
      reqPropVal.prop = (int32_t) ConfigProperty::CCF_IVI_SOCIVIIC_HMI_VEHASSETID;
      getVehicleProperty(reqPropVal, resultPropVal);  
      varient=resultPropVal.value.stringValue.c_str();
      
    if (strcmp(varient.c_str(),"altrozpremce_jul_23") == 0) {
    ALOGD("varient = %s", varient.c_str());
    gnss_varient = 3;
    }
    else if (strcmp(varient.c_str(),"punch_premce_dec_23") == 0) {
    ALOGD("varient = %s", varient.c_str());
    gnss_varient = 4;
    }
    else if ((strcmp(varient.c_str(),"tiago_jun_24") == 0)||(strcmp(varient.c_str(),"tiagoev_aug_24") == 0)) {
    ALOGD("varient = %s", varient.c_str());
    gnss_varient = 5;
    }
    else if ((strcmp(varient.c_str(),"tigor_jun_24") == 0)||(strcmp(varient.c_str(),"tigorev_aug_24") == 0)) {
    ALOGD("varient = %s", varient.c_str());
    gnss_varient = 6;
    }
      ALOGD("[%s] CCF_IVI_SOCIVIIC_HMI_VEHASSETID propId=%#x value=%s", __func__, resultPropVal.prop,resultPropVal.value.stringValue.c_str());
      return gnss_varient;
   }
   
int varconfig_main()
{   ALOGE("Varient info main enter"); 
    //getVehiclePropCallback(status, propValue);
    //getVehicleProperty(vHalPropInVal, vHalPropOutVal);
    int var_info=get_varient();
    return var_info;
}
