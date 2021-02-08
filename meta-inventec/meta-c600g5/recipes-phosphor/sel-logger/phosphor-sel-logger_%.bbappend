FILESEXTRAPATHS_append_c600g5 := "${THISDIR}/${PN}:"

DEPENDS_append_c600g5 = " phosphor-dbus-interfaces"

SRCREV := "761bf202ba9db9fe644f8f400a5e768abe1a70cf"

SRC_URI_append_c600g5 = " file://0001-add-event-log-support.patch \
                          file://xyz.openbmc_project.Logging.IPMI.service \
"

# Enable threshold monitoring
EXTRA_OECMAKE_append = " -DSEL_LOGGER_MONITOR_THRESHOLD_EVENTS=ON \
    -DREDFISH_LOG_MONITOR_EVENTS_LOG=ON \
"

do_install_append_c600g5() {
    install -d ${D}${systemd_unitdir}/system/
    install -m 0644 ${WORKDIR}/xyz.openbmc_project.Logging.IPMI.service \
        ${D}${systemd_unitdir}/system
}

