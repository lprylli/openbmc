FILESEXTRAPATHS_prepend_sv310g4 := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Create-sensor-gen-extra-from-sensor-yaml.patch \
            file://0002-Support-sdr-related-command-handlers.patch \
            file://0003-Get-fru-sdr-from-fru-dbus.patch \
           "

do_install_append(){
  install -d ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/include/ipmid/utils.hpp ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/include/ipmid/types.hpp ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/sensorhandler.hpp ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/selutility.hpp ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/storageaddsel.hpp ${D}${includedir}/phosphor-ipmi-host
  install -d ${D}${includedir}/openbmc
  install -m 0644 -D ${B}/sensor-gen-extra.cpp ${D}${includedir}/openbmc/sensor-gen-extra.cpp
}
