#!/bin/sh

SERVICE="xyz.openbmc_project.Logging.IPMI"
OBJECT="/xyz/openbmc_project/Logging/IPMI"
INTERFACE="xyz.openbmc_project.Logging.IPMI"
METHOD="IpmiSelAddOem"

case "$1" in
    pressed)
        #Add SEL log (Sensor Type Code 14h, Sensor Specific Offset 00)
        busctl call ${SERVICE} ${OBJECT} ${INTERFACE} ${METHOD} sayy "Power_Button" 9 0x20 0x00 0x04 0x14 0xd6 0x6f 0x00 0xff 0xff 0x02
        ;;

    released)
        ;;

    *)
        #For default or other case
        ;;
esac

exit 0
