FILESEXTRAPATHS_prepend := "${THISDIR}/linux-aspeed:"

SRC_URI += "file://c600g5.cfg \
            file://aspeed-bmc-inventec-c600g5.dts;subdir=git/arch/${ARCH}/boot/dts \
           "

PACKAGE_ARCH = "${MACHINE_ARCH}"
