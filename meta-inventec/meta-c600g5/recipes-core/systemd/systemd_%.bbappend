FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

pkg_postinst_ontarget_${PN} () {
    #disable telnet
    systemctl stop xinetd.service
    systemctl disable xinetd.service
}

