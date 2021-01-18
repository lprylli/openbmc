#!/bin/bash

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
