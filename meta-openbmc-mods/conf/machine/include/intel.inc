OBMC_MACHINE_FEATURES += "\
        obmc-phosphor-fan-mgmt \
        obmc-phosphor-chassis-mgmt \
        obmc-phosphor-flash-mgmt \
        obmc-host-ipmi \
        obmc-host-state-mgmt \
        obmc-chassis-state-mgmt \
        obmc-bmc-state-mgmt \
        "

VIRTUAL-RUNTIME_skeleton_workbook = "${MACHINE}-config"
VIRTUAL-RUNTIME_obmc-inventory-manager = "entity-manager"
VIRTUAL-RUNTIME_obmc-led-monitor = ""
VIRTUAL-RUNTIME_obmc-host-state-manager = "intel-chassis-control"
VIRTUAL-RUNTIME_obmc-chassis-state-manager = "intel-chassis-control"
VIRTUAL-RUNTIME_obmc-discover-system-state = "intel-chassis-control"
VIRTUAL-RUNTIME_obmc-settings-mgmt = "settings"

PREFERRED_PROVIDER_virtual/obmc-host-ipmi-hw = "phosphor-ipmi-kcs"
PREFERRED_PROVIDER_virtual/obmc-chassis-mgmt = "packagegroup-intel-apps"
PREFERRED_PROVIDER_virtual/obmc-fan-mgmt = "packagegroup-intel-apps"
PREFERRED_PROVIDER_virtual/obmc-flash-mgmt = "packagegroup-intel-apps"
PREFERRED_PROVIDER_virtual/obmc-system-mgmt = "packagegroup-intel-apps"
PREFERRED_PROVIDER_virtual/obmc-host-ctl ?= ""
PREFERRED_PROVIDER_virtual/obmc-inventory-data = "entity-manager"
PREFERRED_PROVIDER_virtual/phosphor-led-manager-config-native ?= "intel-led-manager-config-native"
#PREFERRED_PROVIDER_virtual/obmc-gpio-monitor ?= "phosphor-gpio-monitor"
