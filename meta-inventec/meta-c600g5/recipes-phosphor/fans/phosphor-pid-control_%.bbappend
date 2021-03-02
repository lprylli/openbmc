FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " file://writePwm.sh \
             file://keepManualPwm.sh \
             file://phosphor-pid-control.sh \
           "

# If fan control service is stopped or there is no pwm that service can use, output default pwm value.
EXTRA_OECONF += "DEFAULT_PWM=90"

inherit obmc-phosphor-systemd
SYSTEMD_SERVICE_${PN} = "phosphor-pid-control.service"

FILES_${PN} += "${bindir}/writePwm.sh"
FILES_${PN} += "${bindir}/keepManualPwm.sh"
FILES_${PN} += "${bindir}/phosphor-pid-control.sh"

do_install_append() {
    install -m 0755 ${WORKDIR}/writePwm.sh ${D}${bindir}/
    install -m 0755 ${WORKDIR}/keepManualPwm.sh ${D}${bindir}/
    install -m 0755 ${WORKDIR}/phosphor-pid-control.sh ${D}${bindir}/
}
