inherit extrausers
EXTRA_USERS_PARAMS_pn-obmc-phosphor-image = " \
  usermod -p '\$1\$KdSxTq5i\$W7g5v8sWXDsF0zvh.8Rk61' root; \
"

OBMC_IMAGE_EXTRA_INSTALL_append_sv310g4 += " ipmitool \
                                             sv310g4-gpio-init \
                                             sv310g4-service-oem \
                                             phosphor-sel-logger \
                                             sv310g4-ipmi-oem \
                                             guid-generator \
                                             entity-manager \
                                             dbus-sensors \
                                             sv310g4-powerctrl \
                                             phosphor-gpio-monitor \
                                             sv310g4-gpio-monitor-register \
                                             wiwynn-interrupt-dbus-register \
                                             button-handler \
                                             phosphor-ipmi-ipmb \
                                             srvcfg-manager \
                                             bios-updater \
                                           "

