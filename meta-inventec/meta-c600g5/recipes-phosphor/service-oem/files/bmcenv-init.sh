#!/bin/bash
# common BMC environment initialization

# Bind the NIC FRU EEPROM
if [ ! -f /sys/devices/platform/ahb/ahb:apb/ahb:apb:bus@1e78a000/1e78a3c0.i2c-bus/i2c-10/10-0050/eeprom ]
then
    echo 10-0050 > /sys/bus/i2c/drivers/at24/bind
    echo "Bind the NIC FRU EEPROM device"
else
    echo "NIC FRU EEPROM device exists!"
fi

# Create a directory to store non-volatile SEL records
if [ ! -d /usr/share/sel ]; then
    mkdir -p /usr/share/sel
fi

if [ ! -f /usr/share/sel/ipmi_sel ]; then
    touch /usr/share/sel/ipmi_sel
fi

# Create symbolic links to the non-volatile SEL records
if [ ! -L /var/log/ipmi_sel ]; then
    ln -s /usr/share/sel/ipmi_sel /var/log/ipmi_sel
fi

if [ ! -L /var/log/ipmi_sel.1 ]; then
    ln -s /usr/share/sel/ipmi_sel.1 /var/log/ipmi_sel.1
fi

if [ ! -L /var/log/ipmi_sel.2 ]; then
    ln -s /usr/share/sel/ipmi_sel.2 /var/log/ipmi_sel.2
fi

if [ ! -L /var/log/ipmi_sel.3 ]; then
    ln -s /usr/share/sel/ipmi_sel.3 /var/log/ipmi_sel.3
fi

# Create an AC-lost event flag
SCU3C=0x1e6e203c
VAL=$(devmem ${SCU3C} 32)
if [ $((${VAL} & 0x01)) -eq 1 ]; then
    mkdir -p /run/openbmc
    touch /run/openbmc/AC-lost@0
    /usr/bin/busctl call "xyz.openbmc_project.Logging.IPMI" "/xyz/openbmc_project/Logging/IPMI" "xyz.openbmc_project.Logging.IPMI" "IpmiSelAddOem" sayy "PSU_AC_Loss" 9 0x20 0x00 0x04 0x09 0xd8 0x6f 0x04 0xff 0xff 0x02
	
    SCU00=0x1e6e2000
    UNLOCK=0x1688A8A8
    devmem ${SCU00} 32 ${UNLOCK}

    VAL=$(( $VAL & ~0x01 ))
    devmem ${SCU3C} 32 ${VAL}
fi

