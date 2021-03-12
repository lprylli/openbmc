FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
	            

SRC_URI += "  file://0001-PATCH-Add-to-support-CPU-and-DIMM-sensors.patch \
              file://0002-Revise-the-judgements-of-host-power-on-and-BIOS-code-complete.patch \
              file://0003-Add-to-support-PSU-sensor.patch \
              file://0004-Support-to-sum-total-PSU-PIN.patch \      
              file://0005-Support-P3V_VBAT-sensor-reading-by-1-hour-interval.patch \    
              file://0006-c600g5-SEL-05-Implement-P12V-sensors.patch \
              file://0007-Implement-MCX-OCP-NIC-Card-Temperature-sensor.patch \
              file://0008-Implement-NVME-temperature-sensors.patch \
            "
DEPENDS += "obmc-libi2c \
            obmc-libmisc \
           "

RDEPENDS_${PN} += "obmc-libi2c \
                   obmc-libmisc \
                  "

SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.voltsensor.service"
SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.nicsensor.service"
SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.nvmesensor.service"

# Disable the CPU sensor Tcontrol threshold setting
EXTRA_OECMAKE_append += " -DBMC_CPU_SENSOR_TCONTROL=OFF"
