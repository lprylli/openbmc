FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
SRC_URI += "file://0001-add-cpld-image-upload-flow.patch\
           "
#add cpld service
SYSTEMD_SERVICE_${PN}-updater += " \
   obmc-cpld-update@.service \
"

