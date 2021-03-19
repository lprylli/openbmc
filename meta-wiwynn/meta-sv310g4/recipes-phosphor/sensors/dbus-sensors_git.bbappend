FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
            file://0001-Revise-the-judgements-of-host-power-on-and-BIOS-code-complete.patch \
            file://0002-Add-to-support-PSU-sensor.patch \
            file://0003-Add-to-support-OCP-NIC-temperature-sensor.patch \
            file://0004-Add-to-support-CPU-and-DIMM-sensors.patch \
            file://0005-Add-to-support-VR-sensor.patch \
            file://0006-Add-to-support-NVMe-sensor.patch \
            file://0007-Support-NIC-card-temperature-over-MCTP.patch \
            file://0008-Add-to-support-miscellaneous-temperature-sensor.patch \
            file://0009-Support-to-sum-total-PSU-PIN.patch \
            file://0010-Set-sensor-hysteresis-to-0.patch \
            file://0011-Fix-invalid-T4-temp-reading.patch \
            "

DEPENDS += "obmc-libi2c \
            obmc-libmisc \
           "

RDEPENDS_${PN} += "obmc-libi2c \
                   obmc-libmisc \
                  "
SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.nicsensor.service"
SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.vrsensor.service"
SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.nvmesensor.service"
SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.miscsensor.service"

# Disable the CPU sensor Tcontrol threshold setting
EXTRA_OECMAKE_append += " -DBMC_CPU_SENSOR_TCONTROL=OFF"
