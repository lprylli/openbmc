FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

EXTRA_OECONF_append_c600g5 = " --enable-negative-errno-on-fail"

CHIPS = " \
        bus@1e78a000/i2c-bus@c0/emc1412@4c \
        pwm-tacho-controller@1e786000 \
        "
ITEMSFMT = "ahb/apb/{0}.conf"

ITEMS = "${@compose_list(d, 'ITEMSFMT', 'CHIPS')}"
ITEMS += "iio-hwmon.conf"

ENVS = "obmc/hwmon/{0}"
SYSTEMD_ENVIRONMENT_FILE_${PN} += " ${@compose_list(d, 'ENVS', 'ITEMS')}"
