#!/bin/bash

function USAGE () {
    echo "============================================================================"
    echo "USAGE: $0 [log_path] [os_version] [test_type]"
    echo "       os_version:o for Android O; p for Android P; y for Yocto Linux"
    echo "       test_type:l for Laboratory test; f for Field test"
    echo "       eg: $0 /mnt/media_rw/C548-A45A o l"
    echo "============================================================================"
}

if [ $# != 3 ] ; then
    echo "ERROR: command parameter is wrong"
    USAGE
    exit 1;
fi

if [ ! -d $1 ];then
    echo "ERROR: log path is not exist:$1"
    USAGE
    exit 1;
else
    log_path=$1
    echo "Log path is $log_path"
fi

os_version=$2

k=0
m=0
s=0
e=0

adr_log_path=$log_path/adr
adrd_log_path=$log_path/adr_log
logcat_path=$log_path/logcat
gps_log_path=$log_path/gps
#gps_log_path=/data/gps
nmea_log_x=1300
nmea_log_y=150
nmea_log_start_x=400

if [ "$os_version"x = "o"x ]; then
    setenforce 0
    gps_debug_prop_file=/data/misc/gps/mnl.prop
    nmea_log_start_y=900
    adr_conf_file=/vendor/etc/adr/adr.conf
    crash_log_path=/data/tombstones
elif [ "$os_version"x = "p"x ]; then
    setenforce 0
    gps_debug_prop_file=/data/vendor/gps/mnl.prop
    nmea_log_start_y=850
    adr_conf_file=/vendor/etc/adr/adr.conf
    crash_log_path=/data/tombstones
elif [ "$os_version"x = "y"x ]; then
    gps_debug_prop_file=/usr/share/gps/mnl.prop
    adr_conf_file=/etc/adr/adr.conf
    crash_log_path=/usr/bin
else
    echo "ERROR: os_version is $os_version, not o or p, or y"
    USAGE
    exit 1;
fi

echo "OS version is $os_version"

if [ "$3"x = "l"x ]; then
    echo "this is Laboratory test"
    sed -i "s?.*open_fake_odom=.*?open_fake_odom=1?" $adr_conf_file
elif [ "$3"x = "f"x ]; then
    echo "this is Field test"
    sed -i "s?.*open_fake_odom=.*?open_fake_odom=0?" $adr_conf_file
else
    echo "ERROR: test_type is $3, not l or f"
    USAGE
    exit 1;
fi

echo "--- Step 1: Clean ADR log files ---"
rm -rf $adr_log_path
rm -rf $adrd_log_path
rm -rf $gps_log_path
mkdir $adr_log_path
mkdir $adrd_log_path
mkdir $gps_log_path
if [ "$os_version"x != "y"x ]; then
    rm -rf $logcat_path
    mkdir $logcat_path
fi

echo "--- Step 2: Prepare config for ADR, stop adrd---"
start_cmd="start"
stop_cmd="stop"
if [ "$os_version"x = "y"x ]; then
    start_cmd="systemctl start"
    stop_cmd="systemctl stop"
fi

$stop_cmd adrd

sed -i "s?.*turn_on_path=.*?turn_on_path=$adr_log_path/adr_archive.dat?" $adr_conf_file
sed -i "s?.*log_path=.*?log_path=$adrd_log_path?" $adr_conf_file
sed -i "s?.*open_data_store=.*?open_data_store=1?" $adr_conf_file
chmod 777 $adr_log_path

echo "--- Step 3: Enable GPS debuglog ---"
echo "debug.dbg2file=1" > $gps_debug_prop_file
echo "debug.filename=$gps_log_path/gpsdebug.log" >> $gps_debug_prop_file
chmod 777 $gps_debug_prop_file
chmod 777 $gps_log_path

# All CPU online
#echo 0 > /proc/hps/enabled
#for CPU in /sys/devices/system/cpu/cpu?; do echo 1 > $CPU/online; done

# All CPU maximum performance
#max_avi_freq=0
#for CPU in /sys/devices/system/cpu/cpu?
#do
#        avi_freqs="$(cat $CPU/cpufreq/scaling_available_frequencies)"
#        echo "$CPU, all available frequencies are $avi_freqs"
#        avi_freqs_arr=($avi_freqs)
#        for avi_freq in ${avi_freqs_arr[@]}
#        do
#                if [ $max_avi_freq -lt $avi_freq ]
#                then
#                        max_avi_freq=$avi_freq
#                fi
#       done
#       echo "$CPU, max available frequency is $max_avi_freq"
#       echo $max_avi_freq > $CPU/cpufreq/scaling_min_freq
#done

if [ "$os_version"x = "y"x ]; then
    echo "--- Step 5: Ignore the system debug message! ---"
    echo 0 > /proc/sys/kernel/printk

    echo "--- Step 6: Set-up Network for PowerGPS connection! ---"
    netconfig=$(ifconfig -a)
    ret=$(echo $netconfig | grep "rndis0")
    if [[ $ret != "" ]];then
	    echo "--- contain rndis0"
	    ifconfig rndis0 192.168.1.100 netmask 255.255.255.0 up
    #else
        #echo "--- not contain rndis0, then use usb0"
	    #cd /sys/kernel/config/usb_gadget
	    #mkdir g1
	    #cd g1/
	    #echo "0x0525" > idVendor
	    #echo "0xa4a2" > idProduct
	    #mkdir functions/rndis.usb0
	    #mkdir configs/c.1
	    #ln -s functions/rndis.usb0/ configs/c.1
	    #echo 11271000.usb > UDC
	    #ifconfig usb0 192.168.1.100 up
    fi

    echo "--- Step 7: Generate core dumped file! ---"
    ulimit -c unlimited

    echo "--- Step 8: restart adrd! ---"
    $start_cmd adrd 
else
    echo "--- Step 4: Enable stp dump debuglog ---"
    mkdir /mnt/sdcard/mtklog
    mkdir /mnt/sdcard/mtklog/gpsdbglog
    echo 14 1 > /proc/driver/wmt_dbg
    stp_dump3&
    sleep 3

    echo "--- Step 5: Enable log process! ---"
    killall logcat
    logcat -G 20M
    logcat -b kernel -r 102400 -n 50 -f $logcat_path/kernel$k &
    logcat -r 102400 -n 50 -f $logcat_path/mainlog$m &
    logcat -b system -r 102400 -n 50 -f $logcat_path/vendor$s &
    logcat -b events -r 102400 -n 50 -f $logcat_path/events$e &

    echo "--- Step 6: restart adrd! ---"
    $start_cmd adrd

    #echo "--- Step 7: Start ADR ---"
    #am start -n com.mediatek.ygps/.YgpsActivity
    #sleep 1
    #input tap $nmea_log_x $nmea_log_y
    #sleep 1
    #input tap $nmea_log_start_x $nmea_log_start_y

    echo "--- Step 8: Start monitor logcat process! ---"
    while :
    do
        pro_kernel=$(ps -ef | grep 'logcat -b kernel' | grep -v grep)
        #echo "kernel $pro_kernel"
        if [ -z "$pro_kernel" ]
        then
            echo 'create kernel logcat process'
            let ++k
            logcat -b kernel -r 102400 -n 50 -f $logcat_path/kernel$k &
        fi

        pro_main=$(ps -ef | grep 'logcat -r' | grep -v grep)
        #echo $pro_main
        if [ -z "$pro_main" ]
        then
            echo 'create main logcat process'
            let ++m
            logcat -r 102400 -n 50 -f $logcat_path/mainlog$m &
        fi

        pro_system=$(ps -ef | grep 'logcat -b system' | grep -v grep)
        #echo $pro_system
        if [ -z "$pro_system" ]
        then
            echo 'create system logcat process'
            let ++s
            logcat -b system -r 102400 -n 50 -f $logcat_path/vendor$s &
        fi

        pro_events=$(ps -ef | grep 'logcat -b events' | grep -v grep)
        if [ -z "$pro_events" ]
        then
            echo 'create events logcat process'
            let ++e
            logcat -b events -r 102400 -n 50 -f $logcat_path/events$e &
        fi
        sleep 60
    done
fi
