FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Add-StartLimitBurst-to-phosphor-reset-host-reboot-at.patch \
            file://0002-Run-power-policy-service-for-AC-lost-event-only.patch \
           "

