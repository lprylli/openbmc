FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Revise-transporthandler-for-some-feature.patch \
            file://0002-Implement-IPMI-Chassis-Control-Diag-Command.patch \
            file://0003-Support-IPMI-SystemBootOption-command.patch \
            file://0004-Fix-ipmi-reset-command-of-chassis-control-failure.patch \
	    file://0005-Get-fru-sdr-from-fru-dbus.patch \
           "
	   

do_install_append(){
  install -d ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/sensorhandler.hpp ${D}${includedir}/phosphor-ipmi-host
  install -m 0644 -D ${S}/selutility.hpp ${D}${includedir}/phosphor-ipmi-host
}
