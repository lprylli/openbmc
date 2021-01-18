SUMMARY = "C600G5 IPMI OEM commands"
DESCRIPTION = "C600G5 IPMI OEM commands"
PR = "r0"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

FILESEXTRAPATHS_prepend := "${THISDIR}:"
SRC_URI = "file://${BPN}/"
S = "${WORKDIR}/${BPN}"

DEPENDS = "boost phosphor-ipmi-host phosphor-logging systemd sdbusplus"
DEPENDS += "nlohmann-json"

inherit cmake obmc-phosphor-ipmiprovider-symlink

EXTRA_OECMAKE=""

LIBRARY_NAMES = "libc600g5oemcmds.so"

HOSTIPMI_PROVIDER_LIBRARY += "${LIBRARY_NAMES}"
NETIPMI_PROVIDER_LIBRARY += "${LIBRARY_NAMES}"

FILES_${PN}_append = " ${libdir}/ipmid-providers/lib*${SOLIBS}"
FILES_${PN}_append = " ${libdir}/host-ipmid/lib*${SOLIBS}"
FILES_${PN}_append = " ${libdir}/net-ipmid/lib*${SOLIBS}"
FILES_${PN}-dev_append = " ${libdir}/ipmid-providers/lib*${SOLIBSDEV}"

do_install_append(){
   install -d ${D}${includedir}/c600g5-ipmi-oem
   install -m 0644 -D ${S}/include/*.hpp ${D}${includedir}/c600g5-ipmi-oem
}
