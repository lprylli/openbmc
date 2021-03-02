FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " file://0001-Disable-to-release-IP-from-DHCP.patch \ 
                        "

pkg_postinst_ontarget_${PN} () {
    #disable telnet
    systemctl stop xinetd.service
    systemctl disable xinetd.service
}

