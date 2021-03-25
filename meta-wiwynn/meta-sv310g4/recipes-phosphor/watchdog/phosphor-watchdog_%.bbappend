FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " file://obmc \
             file://poweron.conf \
             file://phosphor-watchdog@.service \
             file://obmc-enable-host-watchdog@.service \
             file://0001-Support-watchdog2-timeout-and-FRB2-event-log.patch \
           "

WATCHDOG_FMT = "../${WATCHDOG_TMPL}:multi-user.target.wants/${WATCHDOG_TGTFMT}"
