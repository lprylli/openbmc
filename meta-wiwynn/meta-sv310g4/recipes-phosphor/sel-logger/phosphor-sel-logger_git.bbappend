FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

EXTRA_OECMAKE += "-DSEL_LOGGER_MONITOR_THRESHOLD_EVENTS=ON"

# Max size 256k
EXTRA_OECMAKE += "-DMAX_SEL_SIZE=262144"

EXTRA_OECMAKE += "-DALMOST_FULL_PERCENTAGE=75"

SRC_URI += " file://0001-Use-sensor-yaml-MBR-in-threshold-event-sel.patch \
           "
