FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Revise-transporthandler-for-some-feature.patch \
            file://0002-Implement-IPMI-Chassis-Control-Diag-Command.patch \
            file://0003-Support-IPMI-SystemBootOption-command.patch \
            file://0004-Fix-ipmi-reset-command-of-chassis-control-failure.patch \
	    file://0005-Create-sensor-gen-extra-from-sensor-yaml.patch \
            file://0006-Support-sdr-related-command-handlers.patch \
            file://0007-Get-fru-sdr-from-fru-dbus.patch \
	    file://0008-Support-CPU-margin-showing-negative-values.patch \
            file://0009-Support-ownerID-config-in-SDR.patch \
            file://0010-Add-sensor-MRB-value-in-sensor-gen-extra.patch \
            file://0011-Add-the-word-Chassis-PSU-NIC-in-FRU-Device-Descripti.patch \
            file://0012-Implement-DCMI-power-reading-and-limit-commands.patch \
           "
	   
EXTRA_OECONF += "--disable-i2c-whitelist-check"

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
