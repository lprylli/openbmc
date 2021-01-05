#!/bin/sh
# SV310G4 GPIO initialization

# GROUP A
GPIO_A0=0   # I
GPIO_A1=1   # O(H)
GPIO_A2=2   # O(H)
GPIO_A3=3   # I

# GROUP B
GPIO_B0=8   # I
GPIO_B1=9   # I
GPIO_B2=10  # O(H)
GPIO_B3=11  # O(H)
GPIO_B4=12  # I
#GPIO_B5=13  # I
GPIO_B6=14  # I
GPIO_B7=15  # I

# GROUP D
GPIO_D0=24  # I
GPIO_D1=25  # O(H)
GPIO_D2=26  # I
GPIO_D3=27  # O(H)
GPIO_D4=28  # O(L)
GPIO_D5=29  # O(H)
GPIO_D6=30  # I
GPIO_D7=31  # I

# GROUP E
GPIO_E0=32  # I
GPIO_E1=33  # O(H)
GPIO_E2=34  # I
GPIO_E3=35  # O(H)
GPIO_E4=36  # I
GPIO_E5=37  # O(H)
GPIO_E6=38  # O(H)
GPIO_E7=39  # I - PWRGD_PS_PWROK

# GROUP F
GPIO_F0=40  # O(H)
GPIO_F1=41  # I
GPIO_F2=42  # I
GPIO_F3=43  # I
GPIO_F4=44  # I
GPIO_F5=45  # I
GPIO_F6=46  # I
GPIO_F7=47  # I

# GROUP G
GPIO_G0=48  # I
GPIO_G1=49  # I 
GPIO_G2=50  # I
GPIO_G3=51  # I
GPIO_G4=52  # I
GPIO_G5=53  # I
GPIO_G6=54  # I
GPIO_G7=55  # I

# GROUP H
GPIO_H0=56  # O(L)
GPIO_H1=57  # O(H)
GPIO_H2=58  # I
GPIO_H3=59  # O(H)
GPIO_H4=60  # O(L)
GPIO_H5=61  # O(L)
GPIO_H6=62  # I
GPIO_H7=63  # I

# GROUP I
GPIO_I0=64  # I
GPIO_I1=65  # O(L)
GPIO_I2=66  # I
GPIO_I3=67  # I

# GROUP Q
GPIO_Q6=134 # I
GPIO_Q7=135 # O(H)

# GROUP R
GPIO_R1=137 # I
GPIO_R2=138 # I
GPIO_R3=139 # I
GPIO_R4=140 # I
GPIO_R5=141 # I
GPIO_R6=142 # I
GPIO_R7=143 # I

# GROUP S
GPIO_S0=144 # I
GPIO_S1=145 # O(L)
GPIO_S2=146 # I
GPIO_S3=147 # I
GPIO_S4=148 # O(H)
GPIO_S5=149 # O(H)
GPIO_S6=150 # O(H)
GPIO_S7=151 # I

# GROUP T
#GPIO_T0=152 # I
GPIO_T4=156 # I
GPIO_T5=157 # I

# GROUP U
GPIO_U5=165 # I

# GROUP V
GPIO_V1=169 # I

# GROUP Y
GPIO_Y0=192 # I
GPIO_Y1=193 # I
#GPIO_Y2=194 # O(L)
GPIO_Y3=195 # O(H)

# GROUP Z
GPIO_Z0=200 # I
GPIO_Z1=201 # I
GPIO_Z2=202 # O(H) - FM_BMC_PWRBTN_OUT_N
GPIO_Z4=204 # I
GPIO_Z5=205 # I
GPIO_Z6=206 # I
GPIO_Z7=207 # I

# GROUP AA
GPIO_AA0=208 # O(L)
GPIO_AA2=210 # I
GPIO_AA3=211 # O(L)
GPIO_AA4=212 # O(H)
GPIO_AA5=213 # O(L)
GPIO_AA6=214 # I
GPIO_AA7=215 # I

# GROUP AB
GPIO_AB0=216 # O(L)
GPIO_AB1=217 # I
GPIO_AB2=218 # O(H)
GPIO_AB3=219 # I

GPIO_SYSFS=/sys/class/gpio
GPIO_EXPORT=${GPIO_SYSFS}/export
GPIO_BASE=$(cat ${GPIO_SYSFS}/gpio*/base)

gpio_export() {
    local gpio=$(($1 + ${GPIO_BASE}))
    local gpio_path=${GPIO_SYSFS}/gpio${gpio}

    if [ ! -d "${gpio_path}" ]; then
        echo ${gpio} > ${GPIO_EXPORT}
    fi
}

gpio_set_direction() {
    local gpio=$(($1 + ${GPIO_BASE}))
    local dir_path=${GPIO_SYSFS}/gpio${gpio}/direction
    local dir=$2

    if [ -f "${dir_path}" ]; then
        echo ${dir} > ${dir_path}
    fi
}

gpio_set_value() {
    local gpio=$(($1 + ${GPIO_BASE}))
    local val_path=${GPIO_SYSFS}/gpio${gpio}/value
    local val=$2

    if [ -f "${val_path}" ]; then
        echo ${val} > ${val_path}
    fi
}

# direction: in/out
# value: 0/1

# Init GPIO to output high
gpio_out_high="\
    ${GPIO_A1} \
    ${GPIO_A2} \
    ${GPIO_B2} \
    ${GPIO_B3} \
    ${GPIO_D1} \
    ${GPIO_D3} \
    ${GPIO_D5} \
    ${GPIO_E1} \
    ${GPIO_E3} \
    ${GPIO_E5} \
    ${GPIO_E6} \
    ${GPIO_F0} \
    ${GPIO_H1} \
    ${GPIO_H3} \
    ${GPIO_Q7} \
    ${GPIO_S4} \
    ${GPIO_S5} \
    ${GPIO_S6} \
    ${GPIO_Y3} \
    ${GPIO_Z2} \
    ${GPIO_AA4} \
    ${GPIO_AB2} \
"
for i in ${gpio_out_high}
do
    gpioset gpiochip0 ${i}=1
    # gpio_export ${i}
    # gpio_set_direction ${i} out
    # gpio_set_value ${i} 1
done

# Init GPIO to output low
gpio_out_low="\
    ${GPIO_D4} \
    ${GPIO_H0} \
    ${GPIO_H4} \
    ${GPIO_H5} \
    ${GPIO_I1} \
    ${GPIO_S1} \
    ${GPIO_AA0} \
    ${GPIO_AA3} \
    ${GPIO_AA5} \
    ${GPIO_AB0} \
"
for i in ${gpio_out_low}
do
    gpioset gpiochip0 ${i}=0
    # gpio_export ${i}
    # gpio_set_direction ${i} out
    # gpio_set_value ${i} 0
done

# Init GPIO to input
#gpio_in="\
#    ${GPIO_E1} \
#    ${GPIO_E2} \
#    ${GPIO_M5} \
#"
# for i in ${gpio_in}
# do
    # gpio_export ${i}
    # gpio_set_direction ${i} in
# done

exit 0
