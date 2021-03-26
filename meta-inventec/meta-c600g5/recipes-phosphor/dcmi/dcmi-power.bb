SUMMARY = "DCMI Power"
DESCRIPTION = "Implement the power for IPMI DCMI"
PR = "r1"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${INVENTECBASE}/COPYING.apache-2.0;md5=34400b68072d710fecd0a2940a0d1658"

inherit autotools pkgconfig
inherit obmc-phosphor-ipmiprovider-symlink
#inherit pythonnative
inherit python3native
inherit systemd
inherit obmc-phosphor-systemd


S = "${WORKDIR}"

SRC_URI = "file://bootstrap.sh \
           file://configure.ac \
           file://dcmi-power.cpp \
           file://dcmi-power.hpp \
           file://utils.hpp \
           file://LICENSE \
           file://Makefile.am \
           file://dcmi-power.service \
          "

DEPENDS += "autoconf-archive-native"
DEPENDS += "sdbusplus"
DEPENDS += "phosphor-logging"
DEPENDS += "${PYTHON_PN}-sdbus++-native"
DEPENDS += "boost"
DEPENDS += "phosphor-dbus-interfaces"


RDEPENDS_${PN} += " \
        sdbusplus \
        phosphor-logging \
        phosphor-dbus-interfaces \
        "

SYSTEMD_SERVICE_${PN} = "dcmi-power.service"
