FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
	            

SRC_URI += "  file://0001-PATCH-Add-to-support-CPU-and-DIMM-sensors.patch \
              file://0002-Revise-the-judgements-of-host-power-on-and-BIOS-code-complete.patch \
              file://0003-Add-to-support-PSU-sensor.patch \
              file://0004-Support-to-sum-total-PSU-PIN.patch \      
              file://0005-Support-P3V_VBAT-sensor-reading-by-1-hour-interval.patch \    
              file://0006-c600g5-SEL-05-Implement-P12V-sensors.patch \
              file://0007-Implement-MCX-OCP-NIC-Card-Temperature-sensor.patch \
              file://0008-Implement-NVME-temperature-sensors.patch \
              file://0009-Implement-GPGPU-nvidia-T4-Card-temperature-sensor.patch \
              file://0010-c600g5-Add-retry-mechanism-for-p3v_vat.patch \
              file://0011-Support-temp-n-reading-and-threshold-setting.patch \
              file://0012-Implement-NIC-card-temperature-with-MCTP-over-I2C.patch \
              file://0013-Implement-P5V_STBY-sensor.patch \
              file://0014-Implement-event-sensor-for-log-BMC-events.patch \
              file://0015-Implement-BMC-Kernel-Panic-in-SEL-description.patch \
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
SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.i2cdevsensor.service"
SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.adci2csensor.service"
SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.eventsensor.service"

# Disable the CPU sensor Tcontrol threshold setting
EXTRA_OECMAKE_append += " -DBMC_CPU_SENSOR_TCONTROL=OFF"
