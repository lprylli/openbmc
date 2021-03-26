FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"
SRC_URI_append = " file://power-restore-policy.override.yml \
                   file://power-cap.override.yml \
                 "
