#!/vendor/bin/sh

#Copyright (c) 2023 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

if [ -f /sys/devices/soc0/soc_id ]; then
     soc_id=`cat /sys/devices/soc0/soc_id`
else
     soc_id=`cat /sys/devices/system/soc/soc0/id`
fi

#*******************************
#SocID - Device mapping
#    SocID              Device
#    -----              ------
#    362,367            Hana
#    377                Talos
#    405                Poipu
#    460                8295
#    533                8650
#    532                8255
#    534                8775
#    619                8775
#    606                7255
#*******************************

case "$soc_id" in
    "362" | "367" | "405" | "377" )
        ln -sf /vendor/lib/rfsa/adsp/libdspCV_v66_skel.so /data/vendor/fastcv/libdspCV_skel.so
        ln -sf /vendor/lib/rfsa/adsp/libfastcvadsp_v66.so /data/vendor/fastcv/libfastcvadsp.so
        ln -sf /vendor/lib/rfsa/adsp/libfastcvadsp_v66_skel.so /data/vendor/fastcv/libfastcvadsp_skel.so
        ln -sf /vendor/lib/rfsa/adsp/libfastcvdsp_v66_skel.so /data/vendor/fastcv/libfastcvdsp_skel.so
        ln -sf /vendor/lib/rfsa/adsp/libworker_pool_v68.so /data/vendor/fastcv/libworker_pool.so;;
    "460" )
        ln -sf /vendor/lib/rfsa/adsp/libdspCV_v68_skel.so /data/vendor/fastcv/libdspCV_skel.so
        ln -sf /vendor/lib/rfsa/adsp/libfastcvadsp_v68.so /data/vendor/fastcv/libfastcvadsp.so
        ln -sf /vendor/lib/rfsa/adsp/libfastcvadsp_v68_skel.so /data/vendor/fastcv/libfastcvadsp_skel.so
        ln -sf /vendor/lib/rfsa/adsp/libfastcvdsp_v68_skel.so /data/vendor/fastcv/libfastcvdsp_skel.so
        ln -sf /vendor/lib/rfsa/adsp/libworker_pool_v68.so /data/vendor/fastcv/libworker_pool.so;;
    "532" | "534" | "619")
        ln -sf /vendor/lib/rfsa/adsp/libdspCV_v73_skel.so /data/vendor/fastcv/libdspCV_skel.so
        ln -sf /vendor/lib/rfsa/adsp/libfastcvadsp_v73.so /data/vendor/fastcv/libfastcvadsp.so
        ln -sf /vendor/lib/rfsa/adsp/libfastcvadsp_v73_skel.so /data/vendor/fastcv/libfastcvadsp_skel.so
        ln -sf /vendor/lib/rfsa/adsp/libfastcvdsp_v73_skel.so /data/vendor/fastcv/libfastcvdsp_skel.so
        ln -sf /vendor/lib/rfsa/adsp/libworker_pool_v73.so /data/vendor/fastcv/libworker_pool.so;;
    "606" )
        ln -sf /vendor/lib/rfsa/adsp/libdspCV_v75_skel.so /data/vendor/fastcv/libdspCV_skel.so
        ln -sf /vendor/lib/rfsa/adsp/libfastcvadsp_v75.so /data/vendor/fastcv/libfastcvadsp.so
        ln -sf /vendor/lib/rfsa/adsp/libfastcvadsp_v75_skel.so /data/vendor/fastcv/libfastcvadsp_skel.so
        ln -sf /vendor/lib/rfsa/adsp/libfastcvdsp_v75_skel.so /data/vendor/fastcv/libfastcvdsp_skel.so
        ln -sf /vendor/lib/rfsa/adsp/libworker_pool_v75.so /data/vendor/fastcv/libworker_pool.so;;
esac
