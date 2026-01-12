#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <log/log.h>

#include "LedService.h"

using namespace std;

namespace aidl {
namespace vendor {
namespace visteon {
namespace led {

std::shared_ptr<LedService> LedService::sInstance = nullptr;

char buf[64];       // Declare buffer for file path and value
int fd;             // Declare file descriptor

const string GREEN_GPIO_PATH = "/sys/class/gpio/gpio331"; // Green LED
const string RED_GPIO_PATH = "/sys/class/gpio/gpio349"; // Red LED

bool green_ledState = false;
bool red_ledState = false;

std::shared_ptr<LedService> LedService::getInstance(){
    if(sInstance == nullptr){
        sInstance = ndk::SharedRefBase::make<LedService>();
    }
    return sInstance;
}

LedService::LedService(){
  ALOGD("LED Service instantiated");  
  
}

LedService::~LedService(){

  ALOGD("Led service is being destroyed");

}

/*int getValue() {
if(green_ledState){
    GPIO_PATH = GREEN_GPIO_PATH;
  }else
    GPIO_PATH = RED_GPIO_PATH;
  }
    ifstream valueFile(GPIO_PATH + "value");
    int value = -1;
    if (valueFile.is_open()) {
        valueFile >> value;
        valueFile.close();
    
    } else {
        ALOGE("Unable to read value from GPIO331");
    }
    return value;
}*/

void gpioSet(int gpio, int value)
{
    sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
    fd = open(buf, O_WRONLY);
    sprintf(buf, "%d", value);
    write(fd, buf, 1);
    close(fd);
}

ndk::ScopedAStatus LedService::turn_red_led_On() {
ALOGE("[%s] :: Red Led ON", __func__);
  led::red_ledState = true;
  led::gpioSet(349, 1);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus LedService::turn_red_led_Off(){
ALOGE("[%s] :: Red Led OFF", __func__);
    led::red_ledState = false;
      led::gpioSet(349, 0);
      return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus LedService::turn_green_led_On(){
ALOGE("[%s] :: Green Led ON", __func__);
    led::green_ledState = true;
      led::gpioSet(331, 1);
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus LedService::turn_green_led_Off(){
ALOGE("[%s] :: Green Led OFF", __func__);
    led::green_ledState = false;
      led::gpioSet(331, 0);
    return ndk::ScopedAStatus::ok();
}

}  // namespace led
}  // namespace visteon
}  // namespace vendor
}  // namespace aidl
