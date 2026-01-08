
#include <aidl/vendor/visteon/led/BnLedService.h>

namespace aidl::vendor::visteon::led {

class LedService : public BnLedService {

public:
    LedService();
    ~LedService();
    ndk::ScopedAStatus turn_red_led_On() override;
    ndk::ScopedAStatus turn_red_led_Off() override;
    ndk::ScopedAStatus turn_green_led_On() override;
    ndk::ScopedAStatus turn_green_led_Off() override;
    

public:
  void gpioSet(int gpio, int value);
  static std::shared_ptr<LedService> sInstance;
  static std::shared_ptr<LedService> getInstance();
  const std::string GREEN_GPIO_PATH = "/sys/class/gpio/gpio331"; // Green LED
const std::string RED_GPIO_PATH = "/sys/class/gpio/gpio349"; // Red LED

bool green_ledState = false;
bool red_ledState = false;
    
};

} // namespace aidl::android::hardware::led

