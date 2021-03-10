#!/bin/sh

zones=("zone1")
manualFilePathBase="/tmp/fanCtrlManual/"
pwmFilePathBase="/sys/devices/platform/ahb/ahb:apb/1e786000.pwm-tacho-controller/hwmon/**/"

status=$(systemctl is-active phosphor-pid-control.service)
if [ $status == "active" ]; then
    for zone in "${zones[@]}"
    do
    	if ! [ -f "$manualFilePathBase$zone" ]; then
    		busctl set-property xyz.openbmc_project.State.FanCtrl /xyz/openbmc_project/settings/fanctrl/$zone xyz.openbmc_project.Control.Mode Manual b false
    	fi
    done
else
    exit 0
fi

if [ -d "$manualFilePathBase" ]; then
    cd $manualFilePathBase
    for file in * ; do
        while read -r line ; do
            pwmFile=$(echo $line | cut -d " " -f 1)
            pwmValue=$(echo $line | cut -d " " -f 2)
            pwmFilePath=$(echo $pwmFilePathBase$pwmFile)
            echo $pwmValue > $pwmFilePath
        done < $file
    done
fi
