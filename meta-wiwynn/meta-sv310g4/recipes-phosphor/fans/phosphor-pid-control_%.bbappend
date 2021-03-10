FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " file://0001-Modify-pid-algorithm-and-add-debug-mode.patch \
             file://0002-Check-sensor-fan-fail-and-reinitialize-sensors.patch \
             file://0003-Remove-manual-folder-when-service-restart.patch \
             file://0004-Get-tjmax-from-dbus.patch \
             file://0005-When-the-postcomplete-is-high-make-all-fans-speed-up-to-100-percent.patch \
             file://writePwm.sh \
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
