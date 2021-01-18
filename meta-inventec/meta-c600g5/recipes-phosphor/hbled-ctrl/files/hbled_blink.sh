#!/bin/bash

while [ 1 ]
do
#	echo swmod_on > /sys/class/mct/hbled
	gpioset gpiochip0 58=1 
	sleep 0.4
#	echo swmod_off > /sys/class/mct/hbled
	gpioset gpiochip0 58=0
	sleep 0.4
done
