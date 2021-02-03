FILESEXTRAPATHS_prepend_sv310g4 := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Support-HMAC-SHA1-to-Authentication-and-Integrity-Algorithm.patch \
            file://0002-Limit-the-host-console-buffer-size-to-1M.patch \
           "
