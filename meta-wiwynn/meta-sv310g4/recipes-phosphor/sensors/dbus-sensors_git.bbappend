FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
            file://0001-Revise-the-judgements-of-host-power-on-and-BIOS-code-complete.patch \
            file://0002-Add-to-support-PSU-sensor.patch \
            "
