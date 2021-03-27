#!/bin/bash

SERVICE="xyz.openbmc_project.Logging.IPMI"
OBJECT="/xyz/openbmc_project/Logging/IPMI"
INTERFACE="xyz.openbmc_project.Logging.IPMI"
METHOD="IpmiSelAddOem"

/sbin/fw_printenv | grep 'bmc_update'
res=$?

if [ $res -eq 0 ]; then
    busctl call $SERVICE $OBJECT $INTERFACE $METHOD sayy "BMC_FW_UPDATE" 9 0x20 0x00 0x04 0x2b 0xf7 0x6f 0xc7 0x02 0xff 0x02
    /sbin/fw_setenv bmc_update
    /sbin/fw_setenv is_record_update_sel true
    exit 0;
fi

/sbin/fw_printenv | grep 'is_record_update_sel'
res=$?

if [[ ${res} -eq 1 ]]; then
    busctl call $SERVICE $OBJECT $INTERFACE $METHOD sayy "IB_BMC_FW_UPDATE" 9 0x20 0x00 0x04 0x2b 0xf8 0x6f 0x07 0xff 0xff 0x02
    /sbin/fw_setenv is_record_update_sel true
fi

exit 0;

