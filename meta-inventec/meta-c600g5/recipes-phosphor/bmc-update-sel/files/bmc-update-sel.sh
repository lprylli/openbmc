#!/bin/bash

SERVICE="xyz.openbmc_project.Logging.IPMI"
OBJECT="/xyz/openbmc_project/Logging/IPMI"
INTERFACE="xyz.openbmc_project.Logging.IPMI"
METHOD="IpmiSelAdd"
METHOD_OEM="IpmiSelAddOem"

/sbin/fw_printenv | grep 'bmc_update'
res=$?

if [[ ${res} -eq 0 ]]; then
    busctl call $SERVICE $OBJECT $INTERFACE $METHOD ssaybq "SEL Entry" "/xyz/openbmc_project/sensors/versionchange/BMC_FW_UPDATE" 3 {0x07,0x00,0x00} yes 0x20
    /sbin/fw_setenv bmc_update
    /sbin/fw_setenv update_sel_add yes
    exit 0;
fi

/sbin/fw_printenv | grep 'update_sel_add'
res=$?

if [[ ${res} -eq 1 ]]; then
    busctl call ${SERVICE} ${OBJECT} ${INTERFACE} ${METHOD_OEM} sayy "SEL Entry" 9 {0x20,0x0,0x4,0x2b,0xda,0x6f,0x7,0x0,0x0} 0x02
    /sbin/fw_setenv update_sel_add yes
fi

exit 0;
