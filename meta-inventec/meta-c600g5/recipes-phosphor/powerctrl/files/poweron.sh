#!/bin/bash
SERVICE="xyz.openbmc_project.Logging.IPMI"
OBJECT="/xyz/openbmc_project/Logging/IPMI"
INTERFACE="xyz.openbmc_project.Logging.IPMI"
METHOD="IpmiSelAddOem"
PWRBTNACTFLAG="/var/power_btn_flag"

#Add SEL (Sensor Type Code 2Ch, Sensor Specific Offset 02h) when power control via command
if [ ! -f $PWRBTNACTFLAG ]; then
    busctl call ${SERVICE} ${OBJECT} ${INTERFACE} ${METHOD} sayy "FRU_State" 9 0x20 0x00 0x04 0x2c 0xd7 0x6f 0x02 0x01 0xff 0x02
else
    rm -f $PWRBTNACTFLAG
fi

echo "[C600G5][S] System Power on"

pwrstatus=$(busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | cut -d' ' -f2)
if [ ${pwrstatus} -eq 0 ]; then
    /usr/bin/gpioset gpiochip0 35=0
    sleep 1
    /usr/bin/gpioset gpiochip0 35=1
    sleep 1
    # Monitor the PGood Status
    CHECK=0
    while [ ${CHECK} -lt 10 ]
    do
        sleep 1
        pwrstatus=$(busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | cut -d' ' -f2)
        if [ ${pwrstatus} -eq 1 ]; then
            busctl set-property xyz.openbmc_project.Watchdog /xyz/openbmc_project/watchdog/host0 xyz.openbmc_project.State.Watchdog Enabled b false
            if [ $? == 0 ]; then
                break;
            fi
        fi
        echo "${CHECK} - Failed to disable WDT."
        (( CHECK=CHECK+1 ))
    done
    echo "[C600G5][P] System Power on"
else
    echo "[C600G5][P] System Power already on"
fi

exit 0;
