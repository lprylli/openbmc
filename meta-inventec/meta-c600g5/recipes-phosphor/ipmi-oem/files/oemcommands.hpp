

#pragma once

#include <ipmid/api.hpp>

#include <map>
#include <string>

static constexpr ipmi_netfn_t netFnc600g5OEM2 = 0x30;

static const uint8_t oemMaxIPMIWriteReadSize = 50;

enum ipmi_c600g5_oem2_cmds : uint8_t
{
    CMD_SET_FAN_PWM = 0x11,
    CMD_SET_FSC_MODE = 0x12,
    CMD_GET_FSC_MODE = 0x13,
};

enum
{
    FSC_MODE_MANUAL = 0x00,
    FSC_MODE_AUTO = 0x01,
};

// Maintain the request data pwmIndex (which pwm we are going to write).
std::map<uint8_t, std::string> pwmFileTable = {{0x00, "pwm1"}, {0x01, "pwm2"},
                                               {0x02, "pwm3"}, {0x03, "pwm4"},
                                               {0x04, "pwm5"}, {0x05, "pwm6"}};
// Maintain which zone pwm files belong to.
std::map<std::string, std::string> zoneTable = {
    {"pwm1", "zone1"}, {"pwm2", "zone1"}, {"pwm3", "zone1"},
    {"pwm4", "zone1"}, {"pwm5", "zone1"}, {"pwm6", "zone1"}};

constexpr auto parentPwmDir = "/sys/devices/platform/ahb/ahb:apb/"
                              "1e786000.pwm-tacho-controller/hwmon/";
constexpr auto manualModeFilePath = "/tmp/fanCtrlManual/";

struct SetPwmRequest
{
    uint8_t pwmIndex;   // 00h - 05h (pwm1 - pwm6), 06h: for all of fan.
    uint8_t pwmPercent; // 01h - 64h (1 - 100)
};

struct SetFscModeRequest
{
    uint8_t mode; // 00h Manual, 01h Auto
};

struct GetFscModeResponse
{
    uint8_t mode; // 00h Manual, 01h Auto
};
