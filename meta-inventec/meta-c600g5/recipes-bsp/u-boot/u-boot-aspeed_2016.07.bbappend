FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Set-FAN-default-PWM.patch \
                           file://0002-Set-the-default-UART-Route.patch \
           file://0003-Configure-dedicate-LAN-port-LED.patch \
                            "
