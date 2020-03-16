FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Add-to-configure-MaxValue-and-MinValue-in-ADC-sensor.patch \
            file://0003-Add-to-support-VR-sensor-without-VR-driver.patch \
            file://0004-Support-Event-only-type-sensor-monitoring.patch \
            file://0005-Add-Invalid-Value-property-for-power-on-sensors.patch \
            file://0006-IPMB-type-sensors-to-support-the-invalid-value-property.patch \
            file://0009-PSU-type-sensors-to-support-the-invalid-value-property.patch \
            file://0011-Support-cpu-peci-sensors-including-the-invalid-value-property.patch \
            file://0012-Add-to-support-NVMe-temperature-sensor.patch \
            file://0013-Add-to-support-NIC-temperature-sensor.patch \
            file://0015-Refer-to-the-pgood-property-for-powerStatusOn-check.patch \
            file://0019-Tach-type-Fan-sensors-to-support-the-invalid-value-property.patch \
           "

SYSTEMD_SERVICE_${PN} += "xyz.openbmc_project.vrsensor.service \
                          xyz.openbmc_project.eventsensor.service \
                          xyz.openbmc_project.nvmesensor.service \
                          xyz.openbmc_project.nicsensor.service \
                         "

DEPENDS += "obmc-libi2c \
            obmc-libmisc \
           "

RDEPENDS_${PN} += "obmc-libi2c \
                   obmc-libmisc \
                  "
