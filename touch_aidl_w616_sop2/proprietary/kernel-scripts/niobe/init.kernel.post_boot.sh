#=============================================================================
# Copyright (c) 2019-2024 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
#
# Copyright (c) 2009-2012, 2014-2019, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of The Linux Foundation nor
#       the names of its contributors may be used to endorse or promote
#       products derived from this software without specific prior written
#       permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

#Implementing this mechanism to jump to powersave governor if the script is not running
#as it would be an indication for devs for debug purposes.
function configure_vm_parameters() {
	# Set Memory parameters.

	MemTotalStr=`cat /proc/meminfo | grep MemTotal`
	MemTotal=${MemTotalStr:16:8}
	let RamSizeGB="( $MemTotal / 1048576 ) + 1"

	# Set the min_free_kbytes and watermark_scale_factor value
	if [ $RamSizeGB -ge 8 ]; then
		# 8GB
		MinFreeKbytes=11584
		WatermarkScale=40
	elif [ $RamSizeGB -ge 4 ]; then
		# 4GB, 6GB
		MinFreeKbytes=8192
		WatermarkScale=50
	elif [ $RamSizeGB -ge 2 ]; then
		# 2GB, 3GB
		MinFreeKbytes=5792
		WatermarkScale=50
	else
		# 1GB
		MinFreeKbytes=4096
		WatermarkScale=60
	fi

	echo $MinFreeKbytes  > /proc/sys/vm/min_free_kbytes
	echo $WatermarkScale > /proc/sys/vm/watermark_scale_factor

}

configure_vm_parameters

fallback_setting()
{
	governor="powersave"
	for i in `ls -d /sys/devices/system/cpu/cpufreq/policy[0-9]*`
	do
		if [ -f $i/scaling_governor ] ; then
			echo $governor > $i/scaling_governor
		fi
	done
}


if [ -f /sys/devices/soc0/soc_id ]; then
	platformid=`cat /sys/devices/soc0/soc_id`
fi

case "$platformid" in
	"629"|"652")
		#Pass as an argument the max number of clusters supported on the SOC
		/vendor/bin/sh /vendor/bin/init.kernel.post_boot-niobe.sh 2
		;;
	*)
		echo "***WARNING***: Invalid SoC ID\n\t No postboot settings applied!!\n"
		fallback_setting
		;;
esac
