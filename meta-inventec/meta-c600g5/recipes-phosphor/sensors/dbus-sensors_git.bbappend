FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
	            

SRC_URI += "  file://0001-PATCH-Add-to-support-CPU-and-DIMM-sensors.patch \
              file://0002-PATCH-Power-status-check-workaround.patch \
            "
# Disable the CPU sensor Tcontrol threshold setting
EXTRA_OECMAKE_append += " -DBMC_CPU_SENSOR_TCONTROL=OFF"
