FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

# SRCREV = "c0dcf0d3f0a0865ccc9ff3ada2d3e70918377401"

EXTRA_OEMESON += "-Dmotherboard-config-path=/usr/share/entity-manager/configurations/C600G5-MB.json"

SRC_URI += "file://C600G5-MB.json \
            "

do_install_append(){
        install -d ${D}/usr/share/entity-manager/configurations
        install -m 0444 ${WORKDIR}/C600G5-MB.json ${D}/usr/share/entity-manager/configurations
}
