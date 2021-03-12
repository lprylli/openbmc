inherit obmc-phosphor-systemd

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

RDEPENDS_${PN}_append = " openssl-bin bash"

PACKAGECONFIG_append = " reboot-update static-bmc net-bridge aspeed-p2a"

SRC_URI_append += " file://verify_image.sh \
                  "

SYSTEMD_SERVICE_${PN} += " verify-image.service"

EXTRA_OECONF_append = " CPPFLAGS='-DENABLE_PCI_BRIDGE'"


do_install_append() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/verify_image.sh ${D}${bindir}
}
