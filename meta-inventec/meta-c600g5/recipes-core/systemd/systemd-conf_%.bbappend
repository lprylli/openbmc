FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
            file://00-IPv6.network \
           "

do_install_append() {
        install -d ${D}${systemd_unitdir}/network/
        install -m 644 ${WORKDIR}/00-IPv6.network ${D}${systemd_unitdir}/network/
}

FILES_${PN} += "{systemd_unitdir}/network/00-IPv6.network"
