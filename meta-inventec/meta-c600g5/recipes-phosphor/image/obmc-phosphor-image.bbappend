inherit extrausers
EXTRA_USERS_PARAMS_pn-obmc-phosphor-image = " \
  usermod -p '\$1\$KdSxTq5i\$W7g5v8sWXDsF0zvh.8Rk61' root; \
"

OBMC_IMAGE_EXTRA_INSTALL_append_c600g5 +=   " c600g5-gpio-init \
					      c600g5-hbled-ctrl \
					      c600g5-powerctrl \
                                              c600g5-gpio-monitor-register \
                                              c600g5-ipmi-oem \
					      common-service-oem \
					      c600g5-bmc-update-sel \
					      common-ipmi-oem \
					      entity-manager \
                                              dbus-sensors \
					      phosphor-pid-control \
                                              phosphor-sel-logger \
                                              phosphor-ipmi-ipmb \                                              
                                              phosphor-ipmi-flash \
                                              phosphor-ipmi-blobs \
                                              phosphor-image-signing \
                                              phosphor-gpio-monitor \
                                              phosphor-post-code-manager \
                                              phosphor-host-postd \
                                              srvcfg-manager \
                                              ipmitool \
                                              xinetd \
                                              netkit-telnet \
                                            "

