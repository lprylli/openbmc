SUMMARY = "C600G5 event register application for gpio monitor"
PR = "r1"

inherit obmc-phosphor-systemd

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${INVENTECBASE}/COPYING.apache-2.0;md5=34400b68072d710fecd0a2940a0d1658"

DEPENDS += "virtual/obmc-gpio-monitor"
RDEPENDS_${PN} += "virtual/obmc-gpio-monitor"

S = "${WORKDIR}"
SRC_URI += "file://SetPowerGoodPropertyOff.service \
            file://SetPowerGoodPropertyOn.service \
            file://toggle_identify_led.sh \
            file://id-button-pressed.service \
	    file://SetBiosPostCompletePropertyOff.service \
            file://power-button-pressed.service \
            file://power_button_event.sh \
           "

do_install() {
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/toggle_identify_led.sh ${D}${bindir}/toggle_identify_led.sh
        install -m 0755 ${WORKDIR}/power_button_event.sh ${D}${bindir}/power_button_event.sh
}

SYSTEMD_SERVICE_${PN} += "SetPowerGoodPropertyOff.service"
SYSTEMD_SERVICE_${PN} += "SetPowerGoodPropertyOn.service"
SYSTEMD_SERVICE_${PN} += "id-button-pressed.service"
SYSTEMD_SERVICE_${PN} += "SetBiosPostCompletePropertyOff.service"
SYSTEMD_SERVICE_${PN} += "power-button-pressed.service"
