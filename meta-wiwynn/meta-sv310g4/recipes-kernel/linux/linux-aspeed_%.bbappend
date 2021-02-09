FILESEXTRAPATHS_prepend := "${THISDIR}/linux-aspeed:"

SRC_URI += "file://sv310g4.cfg \
            file://aspeed-bmc-wiwynn-sv310g4.dts;subdir=git/arch/${ARCH}/boot/dts \
            file://0001-Read-BMC-MAC-address-from-EEPROM.patch \
            file://0002-Clear-the-i2c-slave-state-when-a-bmc-i2c-master-cmd-times-out.patch \
            file://0003-Support-IceLake-version-for-peci-driver.patch \
            file://0004-Support-CPU-margin-reading.patch \
            file://0005-Export-all-DIMMs-during-initialization.patch \
           "

PACKAGE_ARCH = "${MACHINE_ARCH}"
