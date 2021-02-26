#!/bin/sh

SERVICE="xyz.openbmc_project.Logging.IPMI"
OBJECT="/xyz/openbmc_project/Logging/IPMI"
INTERFACE="xyz.openbmc_project.Logging.IPMI"
METHOD="IpmiSelAddOem"
TIMESTAMP="/var/power_btn_timestamp.log"
POWER_FORCE_TIME=4
PWRBTNACTFLAG="/var/power_btn_flag"
case "$1" in
    pressed)
        touch $TIMESTAMP
        #Get the start time of the power button pushed
        echo $(date +%s) > $TIMESTAMP

        #Add SEL log (Sensor Type Code 14h, Sensor Specific Offset 00)
        busctl call ${SERVICE} ${OBJECT} ${INTERFACE} ${METHOD} sayy "Power_Button" 9 0x20 0x00 0x04 0x14 0xd6 0x6f 0x00 0xff 0xff 0x02
        #Create a power button action flag
        touch $PWRBTNACTFLAG
        ;;

    released)
        end_timestamp=$(date +%s)
        if [ -f $TIMESTAMP ]; then
            start_timestamp=$(<$TIMESTAMP)
            delay=$(($end_timestamp - $start_timestamp))

            if [ $delay -ge $POWER_FORCE_TIME ]; then
                #Add SEL log (Sensor Type Code 22h, Sensor Specific Offset 0Ah)
                busctl call ${SERVICE} ${OBJECT} ${INTERFACE} ${METHOD} sayy "ACPI_Power_State" 9 0x20 0x00 0x04 0x22 0xd4 0x6f 0x0a 0xff 0xff 0x02

                rm -f $TIMESTAMP
            else
                echo "Long press is less than 4s."
            fi
            rm -f $TIMESTAMP
        else
            echo "$TIMESTAMP is not found."
        fi
        ;;

    *)
        #For default or other case
        ;;
esac

exit 0
