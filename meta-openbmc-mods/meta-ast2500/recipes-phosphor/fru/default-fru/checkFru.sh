#!/bin/bash

# this script checks the gpio id and loads the correct baseboard fru
fruFile="/etc/fru/baseboard.fru.bin"
idGpio=( 8 9 10 11 12 53)
result=0
idx=0

if [ -f $fruFile ]; then
    exit 0
fi

cd /etc/fru
for gpio in "${idGpio[@]}"
do
    echo $gpio > /sys/class/gpio/export
    typeset -i value=$(cat /sys/class/gpio/gpio$gpio/value)
    value=$((value << idx))
    result=$((result | value))
    idx=$((idx+1))
done

# wp
if (($result == 62)); then
    cat WilsonPoint.fru.bin > $fruFile
# wc
elif (($result == 45)); then
    cat WilsonCity.fru.bin > $fruFile
# cyp
elif (($result == 60)); then
    cat M50CYP2SB2U.fru.bin > $fruFile
# tnp
elif (($result == 12)); then
    cat D50TNP1SB.fru.bin > $fruFile
# default to wft
else
    cat S2600WFT.fru.bin > $fruFile
fi
