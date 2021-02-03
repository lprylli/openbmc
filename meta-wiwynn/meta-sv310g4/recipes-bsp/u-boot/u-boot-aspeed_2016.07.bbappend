FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " file://0001-Clear-HW-Strap-Register-Bit19.patch \
             file://0002-Set-the-default-UART-route-setting.patch \
           "
