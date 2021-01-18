FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://telnet-xinetd \
         "

do_install_append() {
    install -d  ${D}/etc/xinetd.d/
    install -p -m644 ${WORKDIR}/telnet-xinetd ${D}/etc/xinetd.d/telnet
}

