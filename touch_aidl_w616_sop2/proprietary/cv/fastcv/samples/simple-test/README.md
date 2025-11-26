# Application
* Sample application to downscale a grayscale image, perform corner detection with Harris corner detection algorithm.
* Demonstrate configuration of FastCV to operate on various operation modes.


## Usage
```sh
fastcv_simple_test test_data_directory [-l loops] [-o output_file] [-M operationMode]/[-t targetMode] [-w dstWidth] [-h dstHeight]
       -M integer
       Integer value indicating target.  Valid values are:
          FASTCV_OP_LOW_POWER       = 0                         (LOW POWER Mode)
          FASTCV_OP_CPU_PERFORMANCE = 3                         (CPU Performance mode)
       -t integer
       Integer value indicating extended target Mode.Will be ignored if -M is set. Valid values are:
          Extended DSP target mode  = 0             (DSP)
          Extended GPU target mode  = 0             (GPU)
```
