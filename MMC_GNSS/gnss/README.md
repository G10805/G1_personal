## Android + AG3335 Porting Guide(Date: 20230604)

[TOC]

<div STYLE="page-break-after: always;"></div>

### Architecture

![](./doc/architecture.png)

<div STYLE="page-break-after: always;"></div>

### Release Folder Structure

```
Y:.
├─vendor
│  └─airoha
│      ├─daemon
│      │  └─anld
│      ├─common
│      └─hardware
│          └─interfaces
│              └─gnss
└─device
    └─airoha
        └─common
            └─sepolicy
```

<div STYLE="page-break-after: always;"></div>

### Pins of 3335

| Pin Name                          | Usage                            | ANLD Ready |
| --------------------------------- | -------------------------------- | ---------- |
| 3335 Chip Enable                  | Power on/off AG3335              | √          |
| Host Wakeup 3335 Pin <sup>1</sup> | Host use this pin to wakeup 3335 | √          |
| 3335 Wakeup Host Pin <sup>2</sup> | 3335 use this pin to wakeup host | √          |

<sup>1</sup> This pin is not necessary when use SPI, because slave SPI driver has ability to wakeup CM4.

<sup>2</sup> For testing, we assume that the android board will **NOT** enter sleep mode.
**If use SPI for data communication, this pin should be use to listen for an interrupt from slave**

### Porting Tutorial

#### Merge code to aosp

1. Put folders under "**vendor**" into ${ANDROID_SDK}/vendor

2. Add "**device/airoha/common/sepolicy**" in Android Sepolicy Path.

3. Add the following code in Product Makefiles

 **The default hidl executable file name is 2.0. If you are using Android 11, you need to change it to 2.1**

**For Android 14, change to <u>android.hardware.gnss-service.airoha</u>**

```
PRODUCT_PACKAGES += \
    libcurl \
    android.hardware.gnss@2.0-service-airoha \
    airoha.hardware.anld 
```

4. Config TTY Name(Only For UART) in both xml files.
   
   ```
   // anld_user_setting.xml
       <uart>
         <name>/dev/ttyUSB0</name>
         <!-- Uart Flow Control: none/software/hardware -->
         <flow_control>software</flow_control>
       </uart>
   
   // anld_user_setting_factory.xml
       <uart>
         <name>/dev/ttyUSB0</name>
         <!-- Uart Flow Control: none/software/hardware -->
         <flow_control>software</flow_control>
       </uart>
   
   ```
   
   > If you want to communicate with AG3335 through SPI or I2C, you need to implement a port driver.

5. Implement your own GPIO-Control Function in the following wrapper

Unlike UART, the ways to initial GPIO are different in different platform. It's better for developers to implement GPIO control function base on their specific platform.

<u>${SDK_PATH}\vendor\airoha\daemon\anld\driver\gpio\GPIOControl.cpp</u>

```C
void portPowerOnChip();              // Must Be Implemented
void portPowerOffChip();          // Must Be Implemented
void portGenerateInterrupt();    
void portGnerateInterruptLow();   // Must Be Implemented
void portGenerateInterruptHigh(); // Must Be Implemented
```

PortGnerateInterruptLow() and portGenerateInterruptHigh() control the same gpio(**HOST_WAKEUP_GPS_PIN**), but output different value.

The **HOST** platform is different, so You need to modify the original flow and implement GPIO control according to your platform .

And there is also a simple driver provided in kernel/driver/airoha/. After merged, it will show a special device node named “/dev/airoha_gps” which allow code in user space to access it. 
Writing the corresponding words to the node can the GPIO/Power level. 

```C
    if(strstr(buffer,"OPEN") != NULL){
        gps_chip_enable(1);
    }else if(strstr(buffer,"CLOSE") != NULL){
        gps_chip_enable(0);
    }else if(strstr(buffer,"DI") != NULL){
        gpio_direction_output(AIROHA_HOST_TO_GPS_PIN, 0);
    }else if(strstr(buffer,"DF") != NULL){
        gpio_direction_output(AIROHA_HOST_TO_GPS_PIN, 1);
    }
```

Example DTS node is shown as follow.

```C
    airoha_gps_demo:airoha_gps_demo { 
        status = "okay";
        compatible = "airoha,airoha-gps";
        airoha-ldo-pin = <&gpio3 24 GPIO_ACTIVE_LOW>;
        airoha-host-wakeup-gps-pin = <&gpio3 25 GPIO_ACTIVE_HIGH>;
        airoha-gps-wakeup-host-pin = <&gpio3 26 IRQ_TYPE_EDGE_RISING>;
        lock-sleep-ms = <300>; /* must bigger than 0, when set to 0 ,disable wakeup lock */
    };
```

A _probe function will parser the dts node and set gpio value to context.

```C
int airoha_gps_pdev_to_gps_devices(struct platform_device *pdev,
                                   airoha_gps_device *gdevice) {
    int ret;
    int gpio;
    enum of_gpio_flags flags;
    struct device_node *airoha_gps_node = pdev->dev.of_node;
    // find host wakeup 3335(GPIO26) FALLING_DOWN_TRIGGER
    GPS_LOGD("%s:Parser \n", __FUNCTION__);
    gpio = of_get_named_gpio_flags(airoha_gps_node,
                                   "airoha-host-wakeup-gps-pin", 0, &flags);
    GPS_LOGD("parser host wakeup 3335 gpio:%d\n", gpio);
    if (!gpio_is_valid(gpio)) {
        GPS_LOGD("init gpio error: %d\n", gpio);
        return -ENODEV;
    }
    gdevice->host_wakeup_chip_gpio_number = gpio;
    gdevice->host_wakeup_chip_gpio_value =
        (flags == OF_GPIO_ACTIVE_LOW) ? 0 : 1;
    // find 3335(GPIO24) wakeup host RAISE_EDGE_TRIGGER
    gpio = of_get_named_gpio_flags(airoha_gps_node,
                                   "airoha-gps-wakeup-host-pin", 0, &flags);
    GPS_LOGD("parser 3335 wakeup host gpio:%d\n", gpio);
    if (!gpio_is_valid(gpio)) {
        GPS_LOGD("init gpio error: %d\n", gpio);
        return -ENODEV;
    }
    gdevice->chip_wakeup_host_gpio_number = gpio;
    gdevice->chip_wakeup_host_flag = flags;
#ifdef USE_CHIP_ENABLE_PIN
    gpio =
        of_get_named_gpio_flags(airoha_gps_node, "airoha-ldo-pin", 0, &flags);
    GPS_LOGD("parser 3335 ldo gpio:%d\n", gpio);
    if (!gpio_is_valid(gpio)) {
        GPS_LOGD("init gpio error: %d\n", gpio);
        return -ENODEV;
    }
    gdevice->chip_ldo_gpio_number = gpio;
    gdevice->chip_ldo_gpio_value = (flags == OF_GPIO_ACTIVE_LOW) ? 0 : 1;
#else
    GPS_LOGD("do not use ldo\n");
    gdevice->chip_ldo_gpio_number = -1;
#endif
    gdevice->wakeup_duration_ms = -1;
    of_property_read_u32(airoha_gps_node, "lock-sleep-ms",
                         &gdevice->wakeup_duration_ms);
    GPS_LOGD("wakeup duration %d ms", gdevice->wakeup_duration_ms);
    return 0;
}
```

> There might be a confusion about the pin named chip_ldo_gpio because you cannot find it in our reference design. Actually, this pin is just used to control the power of our EVK. In the customer's design, the code controlling this pin will be replace with the code that actually control the power of GPS chip.

### Debug Tutorial

#### Use PowerGPS

ANLD supports viewing nmea and satellite in PowerGPS

PowerGPS defaultly will connecting 127.0.0.1:127.0.0.1, acting as **client**.

![image-20210329135705843](doc/powergps_setting.png)

So developers need to forware the Windows TCP to Android TCP.

adb forward tcp:7005 tcp:21121  ==> forward windows tcp port 7005 to android tcp port 21121

**21121** is the tcp port configured in anld_user_setting.xml

Click “Connect” and open gps app in android, NMEA sentences will be redirect to PowerGPS.

#### Capture ANLD Log

Developers can use <u>adb logcat</u> to capture anld log. 

Currently the Log Level is set to Debug.

#### Check nmea output

1. Register Nmea listener at app layer, and you can get the nmea.

2. Use 3-party APK.

#### Get Location

ANLD will update location to framework through HIDL, and user could use APP to get location.

### Factory Test Command

#### Preparation

There is a socket bind at **0.0.0.0:3335** which permit user to send PAIR command to 3335 and get response. The following steps can help connect to anld through a PC or smartphone.

1. Connect to ANLD.
   
   a. Make sure that both  your smartphone/PC and Android board have connected to the same Wi-Fi.
   
   b. Or Connect 127.0.0.:3335 if your program is onboard.

2. Use `ifconfig​` to get the ip address of Android Board, such as 192.168.0.3

3. Install a TCP software in smartphone/PC and connect 192.168.0.3:3335

4. send `PANL001` , and check whether you get the version information as response.

**It should be noted that exposing a port at 0.0.0.0 is not safe, so please NEVER enable it in config file unless in factory test.**

#### Send PAIR Command

##### Data Format

For PAIR command, please reference to**《AG3335 API Reference**》

![pair_format](./doc/pair_format.png)

User does not need send Preamble and Checksum. However,  a **Function Response Field** must be appended to the origin DataField.

| Tracker ID | PktType | Datafiled | Func Response |
| ---------- | ------- | --------- | ------------- |
| PAIR       |         |           | 0，1，2         |

Func Response：

0：No need response

1：add response listener

2：remove response listener

There is a CW Test example as below.

<img src="./doc/cw_test.png" title="" alt="cw_test" width="351">

#### Send PANL Command

PANL Command is another type of debug command, which let user can directly do gnss start or gnss stop without Android framework.

| Command | Usage        | Response                   |
| ------- | ------------ | -------------------------- |
| PANL001 | Test Command | ANLD Version               |
| PANL002 | Open Gnss    | Command OK, and 3335 Start |
| PANL003 | Close Gnss   | Command OK, and 3335 Stop  |

### Host Download Firmware Demo

Defaultly, ANLD will trigger download process when boot, except that the previous download process is successful.

The AG3335 firmware path , set in Android.bp, is `${Root}/vendor/airoha/daemon/anld/gnss_demo` 

> Downloader get image information by parsing flash_download.cfg, so do not forget copying the flash_download.cfg after you get the firmware.

Bin file under this path will be copy to `/vendor/etc/gnss/` after compile, and the backup file will be store in /data/vendor/airoha/ when download successfully.

```c++
#define SPI_DA_PATH "/vendor/etc/gnss/slave_da_SPI.bin"
#define IMAGE_RELATIVE_DIR "/vendor/etc/gnss/"
#define IMAGE_RELATIVE_OTA_PATH "/vendor/etc/gnss/ota/"
#define IMAGE_RELATIVE_BACKUP_PATH "/data/vendor/airoha/"
#define IMAGE_GNSS_FLASH_DOWNLOAD_CFG_PATH     "/vendor/etc/gnss/flash_download.cfg"
#define IMAGE_PARTITION_BACKUP_PATH \
    "/data/vendor/airoha/partition_table_backup.bin"
#define IMAGE_BOOTLOADER_BACKUP_PATH \
    "/data/vendor/airoha/ag3335_bootloader_backup.bin"
#define IMAGE_GNSS_DEMO_BACKUP_PATH "/data/vendor/airoha/gnss_demo_backup.bin"
#define IMAGE_GNSS_CONFIG_BACKUP_PATH \
    "/data/vendor/airoha/gnss_config_backup.bin"
```

**The slave_da_UART.bin is placed in ${Root}/tools/**

> Please put AG3335 Firmware under `${Root}/vendor/airoha/daemon/anld/gnss_demo` 

### EPO Aiding

In order to shorten the time of TTFF, ANLD will automatically aiding EPO and UTC to AG3335 when received the aiding request from gps chip.

Developers only need to config the **vendorID**, **ProjectID**, **DeviceID** in XML file.

#### The way how we update EPO file.

1. **ANLD** will trigger a download request for 3-Days EPO files 1 minutes after bootup.
   1. If download successfully, EPO files will be update after 1 days.
   2. If failed, EPO files will be update after 12 hour.
2. At any time when gps start, if EPO files are invalid, a download request will be trigger for QEPO file.

### Configuration

Developers can config some item in anld_user_setting.xml instead of modify the source code.

xml path: ${SDK_PATH}\vendor\airoha\daemon\anld\anld_user_setting.xml

Android AOSP: ${ROOT}\vendor\etc\airoha_config\anld_user_setting.xml

The available configuration items are as follow.

| Item Name                    | Description                                                                                                                                                                                                                                                         |
| ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| output_debug_log             | Default True. <br> If True, 3335 debug log will be output at communication port. when **uart_log_record** is true, those logs will be stored at disk, by which we can analyse performance issue.                                                                    |
| output_binary_log            | Default True. <br> This must be set to true, or ANLD can't parser some necessary data.                                                                                                                                                                              |
| epo_vendor_id                | Set valid vendor id to enable EPO                                                                                                                                                                                                                                   |
| epo_project_name             | Set valid project id to enable EPO                                                                                                                                                                                                                                  |
| epo_device_id                | Set valid device id to enable EPO. set to `_RANDOM_` to use random device id                                                                                                                                                                                        |
| epo_file_path                | Set valid file path to enable EPO                                                                                                                                                                                                                                   |
| debug_port                   | Default 3335. <br> If this value is larger than 0 , ANLD will listen at 0.0.0.0:xxxx which will accept connection of a tcp client. Factory Command can be send to this port.<br />Set to -1 to disable this port<br/>**Only enable it when debug or factory test.** |
| powergps_port                | Default 21121<br />If this value is larger than 0 , ANLD will listen at 0.0.0.0:xxxx which will accept connection of a tcp client. PowerGPS socket use this port<br />Set to -1 to disable this port<br/>**Only enable it when debug or factory test.**             |
| raw_measurement_support      | Default True. <br> If True, ANLD will parser raw measurement. <br >**This Must be set to TRUE, or CTS will failed.**                                                                                                                                                |
| uart_log_record              | Default True. <br> If True, the uart log will be store at /data/vendor/airoha                                                                                                                                                                                       |
| auto_lock_sleep              | Default False.   <br> If True, ANLD will **NOT** send "unlock sleep" to 3335, which let 3335 not enter tickness mode when it is idle.                                                                                                                               |
| auto_download_epo            | Default True.<br />if True, ANLD will Enable auto download epo flow                                                                                                                                                                                                 |
| wake_duration                | Default 1000.<br />How long will anld keep **wakeup** status after data in or data out.                                                                                                                                                                             |
| need_wakeup_before_send_data | Default True <br />If use SPI for data transfer, please set this to False.                                                                                                                                                                                          |

#### Config xml without build

Developers can change setting without building android and flashing again.

1. open terminal

2. ```bash
   adb pull vendor/etc/airoha_config/anld_user_setting.xml .
   ```

3. modify config

4. ```bash
   adb root
   adb remount
   adb push ./anld_user_setting.xml vendor/etc/airoha_config/
   ```

### Communication Driver

#### Select Driver

配置文件目录：${source}\vendor\airoha\daemon\anld\platform\inc\anld_customer_config.h

```C
#define COMMUNICATION_INTERFACE_UART   0
#define COMMUNICATION_INTERFACE_SPI    1
#define COMMUNICATION_INTERFACE_SELECE COMMUNICATION_INTERFACE_UART <== 
```

#### SPI Driver

##### SPI Read

<img title="" src="doc/spi_read.png" alt="spi read" width="293">

##### SPI Write

<img title="" src="doc/spi_write.png" alt="spi read" width="321">

### Build GPS Firmware to Image

GPS Firmware binaries are placed in vendor\airoha\daemon\anld\gnss_demo.

The "gnss_demo" folder need to be created manually.

Then, we use Android.bp to tell the build system to include the specific binary.

Here is an example.

```
prebuilt_etc {
    name: "gnss_demo.bin",
    src: "gnss_demo/gnss_demo.bin",
    sub_dir: "gnss",
    vendor:true,
}
```

Please add a node for the `flash_download.cfg` file and also add nodes for all the files listed within it.

Beside, a **DA binary** is also need for downloading firmware.

Finally, include all the names of `prebuilt_etc` in the "requires" node of `airoha.hardware.anld`.

```
    required:[
        "anld_user_setting.xml", 
        "gnss_config.bin",
        "gnss_demo.bin",
        "partition_table.bin",
        "ag3335_da.bin",
        "bootloader.bin",
        "flash_download.cfg",
    ],
```

#### File List

| Filename             | AG3335 | AG3352 |
| -------------------- | ------ | ------ |
| flash_download.cfg   | √      | √      |
| partition_table.bin  | √      | √      |
| bootloader.bin       | √      | √      |
| gnss_demo.bin        | √      | √      |
| gnss_config.bin      | √      | √      |
| ag3335_da.bin        | √      |        |
| ag3352_da_115200.bin |        | √      |

> ag3335_da.bin is used after GNSS SDK V3.2.2, and it's located at SDK_ROOT/project/ag3335_evk/host_example/host_download_demo_by_airoha_da
> 
> To avoid the issue of a path being too long, it is recommended to shorten the folder's name.
> 
> Extra AG3335_hdl_demo_airoha_da_UART115200_3_20240227.rar and find ag3335_da.bin

> ag3352_da.bin is used after GNSS SDK V3.2.2, and it's located at SDK_ROOT/project/ag3352_evk/host_example/host_download_demo_by_airoha_da
> 
> To avoid the issue of a path being too long, it is recommended to shorten the folder's name.
> 
> Extra AG3352_hdl_demo_airoha_da_UART115200_3_20240311.rar and find ag3352_da.bin

> For version before GNSS SDK V3.2.2, please contact AIROHA.

### Low Baudrate Support

If some machines only support low baud rates, such as 115200bps, we provide corresponding support. Here are the steps to follow:

1. First, please disable support_raw_measurement and output_debug_log in the XML files.

2. Remove gnss_app_send_command("002") from the gnss_app_task_main function in gnss_app.c.

3. Then, you can change the baud rate of the GPS firmware in the configuration tool.

4. Finally, update the baudrate in default_platform.cpp.

```
    mUartComm =
        new UartVirtualDriver(this, AG3335_UART_DEV, 921600, uartFlowControl);
```

> Please not use the macro named "LOW_BAUD" because it has been **deprecated**.

### Notice

Due to privacy policy, we will release anld_service in **libanldservice.a** , which is compiled for arm64/arm platform, if your project use other platform, please let us know and we will release libs base on specific platform.

------

#### Release Note

##### 2020/12/18

1. Default baudrate is 921600

##### 2020/12/25

1. Firmware download demo

2. Send PAIR through TCP socket

##### 2021/02/01

1. Add PANL Command

2. Add configuration file

##### 2021/03/22

1. update ANLD porting  Tutorial
2. Add auto_download_epo config xml

##### 2021/03/29

1. Add PowerGPS support
2. Fix Glonass satellite USED_IN_FIX bug.

##### 2021/06/03

1. Backup firmware when download finish

2. Pack 3335 bin to vendor.img

##### 2021/08/19

1. Add SPI
2. add configuration

##### 2021/11/25

1. Add auto update EPO.

##### 2023/01/17

1. Modify image.

##### 2023/06/04

1. Add notice for powergps_port and debug_port

##### 2024/07/06

1. Resolved an issue where the ELPO would be downloaded multiple times due to a bug.
2. Avoid download ELPO/EPO too much in 12 hours.
3. Reduce log.

##### 2024/07/15

1. Support Low baudrate.
2. Check printable ASCII characters in NMEA.

##### 2024/08/26

1. Support Android 14 AIDL
