FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

# SRCREV = "c0dcf0d3f0a0865ccc9ff3ada2d3e70918377401"

EXTRA_OEMESON += "-Dmotherboard-config-path=/usr/share/entity-manager/configurations/C600G5-MB.json"

SRC_URI += "file://C600G5-MB.json \
            file://Chicony-R550-PSU.json \
            file://Chicony-R800-PSU.json \
            file://blacklist.json \
            file://fan-table.json \
            file://0001-Set-the-boot-order-of-EntityManager-after-FruDevice.patch \
            file://0002-Get-fru-device-from-motherboard-config.patch \
            file://0003-Remove-the-action-of-stopping-the-service.patch \
            "

do_install_append(){
        install -d 0755 ${D}/usr/share/entity-manager/configurations
        install -m 0444 ${WORKDIR}/C600G5-MB.json ${D}/usr/share/entity-manager/configurations
        install -m 0444 ${WORKDIR}/Chicony-R550-PSU.json ${D}/usr/share/entity-manager/configurations
        install -m 0444 ${WORKDIR}/Chicony-R800-PSU.json ${D}/usr/share/entity-manager/configurations
        install -m 0444 ${WORKDIR}/blacklist.json ${D}/usr/share/entity-manager
	install -m 0444 ${WORKDIR}/fan-table.json ${D}/usr/share/entity-manager/configurations
}
