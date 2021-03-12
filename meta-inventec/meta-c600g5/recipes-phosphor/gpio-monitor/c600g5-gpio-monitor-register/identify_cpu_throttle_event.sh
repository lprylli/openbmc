#!/bin/sh

SERVICE="xyz.openbmc_project.Logging.IPMI"
OBJECT="/xyz/openbmc_project/Logging/IPMI"
INTERFACE="xyz.openbmc_project.Logging.IPMI"
METHOD="IpmiSelAddOem"

#Check Power Good status
pwrstatus=$(busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | cut -d' ' -f2)

if [[ ${pwrstatus} -eq 1 ]]; then
    case "$1" in
        throttle)
            #CPU_Throttle
            echo "Throttle handled -- $3"
            SENNUM1=0x9b
            SENNUM2=0x9c
            SIGNATURE1="CPU0_Status"
            SIGNATURE2="CPU1_Status"
            #Add SEL log (Sensor Type Code 07h, Sensor Specific Offset 0Ah - Processot Auto Throttled)
            SENTYPE=0x07
            EVENTDATA1=0x0a
            ;;

        *)
            #For default or other case
            ;;
    esac

    if [[ "$3" == "assert" ]]; then
        EVENTTYPE=0x6f
    else
        EVENTTYPE=0xef
    fi

    busctl call ${SERVICE} ${OBJECT} ${INTERFACE} ${METHOD} sayy ${SIGNATURE1} 9 0x20 0x00 0x04 ${SENTYPE} ${SENNUM1} ${EVENTTYPE} ${EVENTDATA1} 0xff 0xff 0x02
    busctl call ${SERVICE} ${OBJECT} ${INTERFACE} ${METHOD} sayy ${SIGNATURE2} 9 0x20 0x00 0x04 ${SENTYPE} ${SENNUM2} ${EVENTTYPE} ${EVENTDATA1} 0xff 0xff 0x02
else
    echo "The current power state is power off."
fi

exit 0
