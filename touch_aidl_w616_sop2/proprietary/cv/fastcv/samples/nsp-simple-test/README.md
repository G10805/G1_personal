# About
Sample FastCV NSP pipeline to demonstrate color conversion from UYVY to RGB888 and down scale.
It will read a uyvy raw input image, converts to RGB888 and downscales, stores the generated output to specified file.

For more details on available APIs, refer fastcv.h

## Execution instructions
1. Copy fastcvNSPSimpleTest to device /vendor/bin/
2. Copy libfastcvNSPSimpleTest_skel.so to device /vendor/lib/rfsa/adsp/
3. Copy test data to device.
   * Copy from <FASTCV_NSP_SIMPLE_TEST_DIR>/data/img1920x1020.uyvy /data/
4. Run below commands in device:
* For usage:
```sh
fastcvNSPSimpleTest --help
```
* To excecute:
```sh
fastcvNSPSimpleTest -in /data/img1920x1020.uyvy -out /data/out.rgb -iW 1920 -iH 1020 -oW 960 -oH 510 -l 100
```
