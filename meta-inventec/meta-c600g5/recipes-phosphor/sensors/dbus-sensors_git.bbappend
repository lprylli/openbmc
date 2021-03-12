FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
	            

SRC_URI += "  file://0001-PATCH-Add-to-support-CPU-and-DIMM-sensors.patch \
              file://0002-Revise-the-judgements-of-host-power-on-and-BIOS-code-complete.patch \
              file://0003-Add-to-support-PSU-sensor.patch \
              file://0004-Support-to-sum-total-PSU-PIN.patch \      
              file://0005-Support-P3V_VBAT-sensor-reading-by-1-hour-interval.patch \    
            "
# Disable the CPU sensor Tcontrol threshold setting
EXTRA_OECMAKE_append += " -DBMC_CPU_SENSOR_TCONTROL=OFF"
