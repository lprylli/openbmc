#!/bin/bash
SERVICE="xyz.openbmc_project.Logging.IPMI"
OBJECT="/xyz/openbmc_project/Logging/IPMI"
INTERFACE="xyz.openbmc_project.Logging.IPMI"
METHOD="IpmiSelAddOem"
PWRBTNACTFLAG="/var/power_btn_flag"

#Add SEL (Sensor Type Code 2Ch, Sensor Specific Offset 05h) when power control via command
if [ ! -f $PWRBTNACTFLAG ]; then
    busctl call ${SERVICE} ${OBJECT} ${INTERFACE} ${METHOD} sayy "FRU_State" 9 0x20 0x00 0x04 0x2c 0xd7 0x6f 0x05 0x01 0xff 0x02
else
    rm -f $PWRBTNACTFLAG
fi

echo "[C600G5][S] System Power off"

pwrstatus=$(busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | cut -d' ' -f2)
if [ ${pwrstatus} -eq 1 ]; then
    /usr/bin/gpioset gpiochip0 35=0
    sleep 6
    /usr/bin/gpioset gpiochip0 35=1
    sleep 1
    obmcutil chassisoff
fi

echo "[C600G5][P] System Power off"
exit 0;
