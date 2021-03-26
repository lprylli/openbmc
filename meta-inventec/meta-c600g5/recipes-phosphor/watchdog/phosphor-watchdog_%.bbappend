FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " file://obmc \
             file://poweron.conf \
             file://phosphor-watchdog@.service \
             file://obmc-enable-host-watchdog@.service \
	     file://0001-Add-IPMI-WDT-and-FRB2-SEL-entries-support.patch \
           "

WATCHDOG_FMT = "../${WATCHDOG_TMPL}:multi-user.target.wants/${WATCHDOG_TGTFMT}"
