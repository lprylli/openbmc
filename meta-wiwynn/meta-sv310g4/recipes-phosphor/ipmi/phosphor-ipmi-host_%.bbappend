FILESEXTRAPATHS_prepend_sv310g4 := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Create-sensor-gen-extra-from-sensor-yaml.patch \
            file://0002-Support-sdr-related-command-handlers.patch \
            file://0003-Get-fru-sdr-from-fru-dbus.patch \
            file://0004-Support-IPMI-power-reset-and-power-soft-commands.patch \
            file://0005-Support-CPU-margin-showing-negative-values.patch \
            file://0006-Support-ownerID-config-in-SDR.patch \
            file://0007-Add-sensor-MRB-value-in-sensor-gen-extra.patch \
            file://master_write_read_white_list.json \
           "

FILES_${PN} += " ${datadir}/ipmi-providers/master_write_read_white_list.json \
               "

do_install_append(){
  install -d ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/include/ipmid/utils.hpp ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/include/ipmid/types.hpp ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/sensorhandler.hpp ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/selutility.hpp ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/storageaddsel.hpp ${D}${includedir}/phosphor-ipmi-host
  install -d ${D}${datadir}/ipmi-providers
  install -m 0644 -D ${WORKDIR}/master_write_read_white_list.json ${D}${datadir}/ipmi-providers/master_write_read_white_list.json
  install -d ${D}${includedir}/openbmc
  install -m 0644 -D ${B}/sensor-gen-extra.cpp ${D}${includedir}/openbmc/sensor-gen-extra.cpp
}
