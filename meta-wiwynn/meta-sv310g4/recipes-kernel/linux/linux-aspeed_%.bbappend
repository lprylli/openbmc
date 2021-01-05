FILESEXTRAPATHS_prepend := "${THISDIR}/linux-aspeed:"

SRC_URI += "file://sv310g4.cfg \
            file://aspeed-bmc-wiwynn-sv310g4.dts;subdir=git/arch/${ARCH}/boot/dts \
           "

PACKAGE_ARCH = "${MACHINE_ARCH}"
