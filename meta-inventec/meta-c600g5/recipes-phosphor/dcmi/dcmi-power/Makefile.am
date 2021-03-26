sbin_PROGRAMS = dcmi-power

dcmi_power_SOURCES = dcmi-power.cpp
dcmi_power_LDFLAGS = $(SYSTEMD_LIBS) \
                         $(SDBUSPLUS_LIBS) \
                         -pthread \
                         $(PTHREAD_LIBS) \
                         $(BOOST_CPPFLAGS)
dcmi_power_CFLAGS = $(SYSTEMD_CFLAGS) \
                        $(SDBUSPLUS_CFLAGS) \
                        -pthread \
                        $(PTHREAD_CFLAGS) \
                        $(BOOST_CPPFLAGS)