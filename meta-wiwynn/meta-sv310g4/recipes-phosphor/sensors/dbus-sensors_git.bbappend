FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
            file://0001-Revise-the-judgements-of-host-power-on-and-BIOS-code-complete.patch \
            file://0002-Add-to-support-PSU-sensor.patch \
            file://0003-Add-to-support-OCP-NIC-temperature-sensor.patch \
            file://0004-Add-to-support-CPU-and-DIMM-sensors.patch \
            file://0005-Add-to-support-VR-sensor.patch \
            "

DEPENDS += "obmc-libi2c \
            obmc-libmisc \
           "

RDEPENDS_${PN} += "obmc-libi2c \
                   obmc-libmisc \
                  "
SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.nicsensor.service"
SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.vrsensor.service"

# Disable the CPU sensor Tcontrol threshold setting
EXTRA_OECMAKE_append += " -DBMC_CPU_SENSOR_TCONTROL=OFF"
