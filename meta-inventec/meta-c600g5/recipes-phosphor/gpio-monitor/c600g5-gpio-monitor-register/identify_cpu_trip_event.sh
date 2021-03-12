#!/bin/sh

SERVICE="xyz.openbmc_project.Logging.IPMI"
OBJECT="/xyz/openbmc_project/Logging/IPMI"
INTERFACE="xyz.openbmc_project.Logging.IPMI"
METHOD="IpmiSelAddOem"

#Check Power Good status
#pwrstatus=$(busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | cut -d' ' -f2)
pwrstatus=1

if [[ ${pwrstatus} -eq 1 ]]; then
    case "$1" in
        thermtrip)
            #Add SEL log (Sensor Type Code 07h, Sensor Specific Offset 01h - Thermal Trip)
            echo "Thermtrip handled"
            SENTYPE=0x07
            EVENTDATA1=0x01
            if [[ "$2" == 0 ]]; then
                #CPU0_Thermal_Trip
                cpu0Temp=$(busctl get-property xyz.openbmc_project.CPUSensor /xyz/openbmc_project/sensors/temperature/CPU0_Temp xyz.openbmc_project.Sensor.Value Value | cut -d' ' -f2 | cut -d "." -f 1)
                cpu0Warning=$(busctl get-property xyz.openbmc_project.CPUSensor /xyz/openbmc_project/sensors/temperature/CPU0_Temp xyz.openbmc_project.Sensor.Threshold.Warning WarningHigh | cut -d' ' -f2)
                if [[ "$cpu0Temp" == nan ]]; then
                   echo "Error: cpu temp = nan"
                   exit 0
                fi
                echo "cpu0Temp = ${cpu0Temp}"
                echo "cpu0Warning = ${cpu0Warning}"
                if [[ ${cpu0Temp} -lt ${cpu0Warning} ]]; then
                    echo "Error: cpu0Temp too low"
                    exit 0
                fi
                SENNUM=0x9b
                SIGNATURE="CPU0_Status"
            elif [[ "$2" == 1 ]]; then
                #CPU1_Thermal_Trip
                cpu1Temp=$(busctl get-property xyz.openbmc_project.CPUSensor /xyz/openbmc_project/sensors/temperature/CPU1_Temp xyz.openbmc_project.Sensor.Value Value | cut -d' ' -f2 | cut -d "." -f 1)
                cpu1Warning=$(busctl get-property xyz.openbmc_project.CPUSensor /xyz/openbmc_project/sensors/temperature/CPU1_Temp xyz.openbmc_project.Sensor.Threshold.Warning WarningHigh | cut -d' ' -f2)
                if [[ "$cpu1Temp" == nan ]]; then
                   echo "Error: cpu temp = nan"
                   exit 0
                fi
                echo "cpu1Temp = ${cpu1Temp}"
                echo "cpu1Warning = ${cpu1Warning}"
                if [[ ${cpu1Temp} -lt ${cpu1Warning} ]]; then
                    echo "Error: cpu1Temp too low"
                    exit 0
                fi
                SENNUM=0x9c
                SIGNATURE="CPU1_Status"
            fi
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

    busctl call ${SERVICE} ${OBJECT} ${INTERFACE} ${METHOD} sayy ${SIGNATURE} 9 0x20 0x00 0x04 ${SENTYPE} ${SENNUM} ${EVENTTYPE} ${EVENTDATA1} 0xff 0xff 0x02
else
    echo "The current power state is power off."
fi

exit 0
