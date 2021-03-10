#!/bin/sh

fanTablePath="/usr/share/entity-manager/configurations/fan-table.json"

# pgood=$(busctl call org.openbmc.control.Power /org/openbmc/control/power0 org.freedesktop.DBus.Properties Get ss org.openbmc.control.Power pgood | cut -d" " -f 3)
# if [ $pgood -eq 1 ]; then
	postcomplete=$(busctl call org.openbmc.control.Power /org/openbmc/control/power0 org.freedesktop.DBus.Properties Get ss org.openbmc.control.PostComplete postcomplete | cut -d" " -f 3)
	if [ $postcomplete -eq 0 ]; then
		if [ -f "$fanTablePath" ]; then
			/usr/bin/swampd -c $fanTablePath
		else
			echo "Fan table does not exist, fan output 100% pwm"
			writePwm.sh -p 100
			systemctl stop phosphor-pid-control.service
		fi
	else
		echo "Host is on, not post complete yet fan output 100% pwm"
		writePwm.sh -p 100
		systemctl stop phosphor-pid-control.service
	fi
# else
	# echo "Host is off, fan output 0% pwm"
    # writePwm.sh -p 0
    # systemctl stop phosphor-pid-control.service
# fi
