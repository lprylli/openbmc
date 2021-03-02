

#include "oemcommands.hpp"

#include "openbmc/libobmci2c.h"

#include "Utils.hpp"
#include "xyz/openbmc_project/Common/error.hpp"
#include "xyz/openbmc_project/Control/Power/RestorePolicy/server.hpp"
#include "xyz/openbmc_project/Led/Physical/server.hpp"

#include <systemd/sd-journal.h>

#include <boost/container/flat_map.hpp>
#include <boost/process/child.hpp>
#include <boost/process/io.hpp>
#include <ipmid/utils.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message/types.hpp>
#include <sdbusplus/server/object.hpp>
#include <sdbusplus/timer.hpp>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <variant>
#include <vector>
#include <gpiod.h>

static void register_oem_functions() __attribute__((constructor));

/**
 *  @brief Function of setting fan speed duty.
 *  @brief NetFn: 0x30, Cmd: 0x11
 *
 *  @param[in] pwmIndex - Index of PWM.
 *  @param[in] pwmValue - PWM value for the specified index.
 *
 *  @return Size of command response - Completion Code.
 **/
ipmi_ret_t IpmiSetPwm(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                      ipmi_request_t request, ipmi_response_t response,
                      ipmi_data_len_t dataLen, ipmi_context_t context)
{
    if (*dataLen != sizeof(SetPwmRequest))
    {
        sd_journal_print(LOG_ERR, "[%s] invalid data length %d\n", __FUNCTION__,
                         *dataLen);
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    auto req = reinterpret_cast<SetPwmRequest*>(request);

    if (req->pwmIndex > pwmFileTable.size())
    {
        sd_journal_print(LOG_ERR, "[%s] invalid pwm index %d (0 ~ %d)\n",
                         __FUNCTION__, req->pwmIndex, pwmFileTable.size());
        return IPMI_CC_PARM_OUT_OF_RANGE;
    }

    if (req->pwmPercent < 10 || req->pwmPercent > 100)
    {
        sd_journal_print(LOG_ERR, "[%s] invalid pwm percent %d (10 ~ 100)\n",
                         __FUNCTION__, req->pwmPercent);
        return IPMI_CC_PARM_OUT_OF_RANGE;
    }

    // Check FSC mode is manual mode.
    if (std::filesystem::exists(manualModeFilePath) == false)
    {
        sd_journal_print(LOG_ERR, "[%s] Can't set pwm in auto mode\n",
                         __FUNCTION__);
        return IPMI_CC_NOT_SUPPORTED_IN_PRESENT_STATE;
    }

    // Get pwm file folder.
    auto pwmDirVec = getDirFiles(parentPwmDir, "hwmon[0-9]+");
    if (pwmDirVec.size() == 0)
    {
        sd_journal_print(LOG_ERR, "[%s] failed to pwm file directory.\n",
                         __FUNCTION__);
        return IPMI_CC_UNSPECIFIED_ERROR;
    }
    else if (pwmDirVec.size() > 1)
    {
        sd_journal_print(LOG_ERR, "[%s] found more than one pwm folder.\n",
                         __FUNCTION__);
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    for (const auto& pwmFile : pwmFileTable)
    {
        if (req->pwmIndex == pwmFile.first ||
            req->pwmIndex == pwmFileTable.size())
        {
            // Write pwm to pwm file.
            auto pwmFilePath = pwmDirVec[0] + "/" + pwmFile.second;
            std::ofstream fPwm(pwmFilePath);
            if (fPwm.fail())
            {
                sd_journal_print(LOG_ERR, "[%s] failed to open pwm file %s\n",
                                 __FUNCTION__, pwmFilePath.c_str());
                return IPMI_CC_UNSPECIFIED_ERROR;
            }
            uint64_t scaledPwmValue = req->pwmPercent * 255 / 100;
            fPwm << static_cast<uint64_t>(scaledPwmValue);
            if (fPwm.fail())
            {
                sd_journal_print(LOG_ERR, "[%s] failed to write pwm value %s\n",
                                 __FUNCTION__, pwmFilePath.c_str());
                fPwm.close();
                return IPMI_CC_UNSPECIFIED_ERROR;
            }
            fPwm.close();

            // Write pwm information to manual file.
            auto tmpFilePath = manualModeFilePath + "/tmp"s;
            // Create temporary file to store new manual file text.
            std::ofstream tmpFile(tmpFilePath);
            if (tmpFile.fail())
            {
                sd_journal_print(LOG_ERR, "[%s] failed to open tmp file\n",
                                 __FUNCTION__);
                return IPMI_CC_UNSPECIFIED_ERROR;
            }

            auto zoneStr = zoneTable[pwmFileTable[pwmFile.first]];
            auto manualFilePath = manualModeFilePath + "/"s + zoneStr;
            std::ifstream manualFile(manualFilePath);
            if (manualFile.fail())
            {
                tmpFile.close();
                sd_journal_print(LOG_ERR, "[%s] failed to open manual file.\n",
                                 __FUNCTION__);
                return IPMI_CC_UNSPECIFIED_ERROR;
            }

            std::string line;
            while (std::getline(manualFile, line))
            {
                if (line.find(pwmFile.second) != std::string::npos)
                {
                    tmpFile << pwmFile.second << " " << scaledPwmValue << "\n";
                }
                else
                {
                    tmpFile << line << "\n";
                }

                if (tmpFile.fail())
                {
                    sd_journal_print(LOG_ERR, "[%s] failed to read tmp file\n",
                                     __FUNCTION__);
                    tmpFile.close();
                    manualFile.close();
                    return IPMI_CC_UNSPECIFIED_ERROR;
                }
            }

            tmpFile.close();
            manualFile.close();

            if (std::rename(tmpFilePath.c_str(), manualFilePath.c_str()) != 0)
            {
                sd_journal_print(LOG_ERR, "[%s] failed to rename, errno = %s\n",
                                 __FUNCTION__, strerror(errno));
                return IPMI_CC_UNSPECIFIED_ERROR;
            }
        }
    }

    *dataLen = 0;
    return IPMI_CC_OK;
}

/**
 *  @brief Function of setting fan speed control mode.
 *  @brief NetFn: 0x30, Cmd: 0x12
 *
 *  @param[in] ControlMode - Manual mode or auto mode.
 *
 *  @return Byte 1: Completion Code.
 **/
ipmi_ret_t IpmiSetFscMode(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                          ipmi_request_t request, ipmi_response_t response,
                          ipmi_data_len_t dataLen, ipmi_context_t context)
{
    if (*dataLen != sizeof(SetFscModeRequest))
    {
        sd_journal_print(LOG_ERR, "[%s] invalid data length %d\n", __FUNCTION__,
                         *dataLen);
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    auto req = reinterpret_cast<SetFscModeRequest*>(request);

    std::variant<bool> value;
    if (req->mode == FSC_MODE_MANUAL)
    {
        // Get pwm file folder.
        auto pwmDirVec = getDirFiles(parentPwmDir, "hwmon[0-9]+");
        if (pwmDirVec.size() == 0)
        {
            sd_journal_print(LOG_ERR, "[%s] failed to pwm file directory.\n",
                             __FUNCTION__);
            return IPMI_CC_UNSPECIFIED_ERROR;
        }
        else if (pwmDirVec.size() > 1)
        {
            sd_journal_print(LOG_ERR, "[%s] found more than one pwm folder.\n",
                             __FUNCTION__);
            return IPMI_CC_UNSPECIFIED_ERROR;
        }

        // Get pwm file paths.
        auto pwmFilePathVec = getDirFiles(pwmDirVec[0], "pwm[0-9]+");
        if (pwmFilePathVec.size() == 0)
        {
            sd_journal_print(LOG_ERR, "[%s] failed to get pwm file path.\n",
                             __FUNCTION__);
            return IPMI_CC_UNSPECIFIED_ERROR;
        }

        // Recreate manual files.
        std::filesystem::remove_all(manualModeFilePath);
        std::filesystem::create_directories(manualModeFilePath);
        for (const auto& path : pwmFilePathVec)
        {
            // Get current pwm value.
            std::ifstream fPwm(path);
            if (fPwm.fail())
            {
                sd_journal_print(LOG_ERR, "[%s] failed to open pwm file %s\n",
                                 __FUNCTION__, path.c_str());
                return IPMI_CC_UNSPECIFIED_ERROR;
            }
            int64_t pwmValue = 0;
            fPwm >> pwmValue;
            if (fPwm.fail())
            {
                sd_journal_print(LOG_ERR, "[%s] failed to read pwm value %s\n",
                                 __FUNCTION__, path.c_str());
                fPwm.close();
                return IPMI_CC_UNSPECIFIED_ERROR;
            }
            fPwm.close();

            // Write current pwm to manual file.
            auto fileName = path.substr(path.find_last_of("/") + 1);
            auto zoneFind = zoneTable.find(fileName);
            std::ofstream fManual(manualModeFilePath + zoneFind->second,
                                  std::ofstream::out | std::ofstream::app);
            if (fManual.fail())
            {
                sd_journal_print(LOG_ERR, "[%s] failed to open manual file\n",
                                 __FUNCTION__);
                return IPMI_CC_UNSPECIFIED_ERROR;
            }
            fManual << fileName << " " << pwmValue << "\n";
            if (fManual.fail())
            {
                fManual.close();
                sd_journal_print(LOG_ERR, "[%s] failed to write manual file\n",
                                 __FUNCTION__);
                return IPMI_CC_UNSPECIFIED_ERROR;
            }
            fManual.close();
        }

        value = true;
    }
    else if (req->mode == FSC_MODE_AUTO)
    {
        std::filesystem::remove_all(manualModeFilePath);
        value = false;
    }
    else
    {
        sd_journal_print(LOG_ERR, "[%s] invalid mode %d\n", __FUNCTION__,
                         req->mode);
        return IPMI_CC_PARM_OUT_OF_RANGE;
    }

    constexpr auto modeService = "xyz.openbmc_project.State.FanCtrl";
    constexpr auto modeRoot = "/xyz/openbmc_project/settings/fanctrl";
    constexpr auto modeIntf = "xyz.openbmc_project.Control.Mode";
    constexpr auto propIntf = "org.freedesktop.DBus.Properties";

    // Bus for system control.
    std::shared_ptr<sdbusplus::asio::connection> bus = getSdBus();

    // Get all zones object path.
    DbusSubTree zonesPath = getSubTree(*bus, modeRoot, 1, modeIntf);
    if (zonesPath.empty() == true)
    {
        sd_journal_print(LOG_ERR, "[%s] failed to get zones object path.\n",
                         __FUNCTION__);
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    for (auto& path : zonesPath)
    {
        auto msg = bus->new_method_call(modeService, path.first.c_str(),
                                        propIntf, "Set");
        msg.append(modeIntf, "Manual", value);

        try
        {
            bus->call_noreply(msg);
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            sd_journal_print(LOG_ERR, "[%s] failed to set fan mode %s.\n",
                             __FUNCTION__, e.what());
            return IPMI_CC_RESPONSE_ERROR;
        }
    }

    *dataLen = 0;
    return IPMI_CC_OK;
}


/**
 *  @brief Function of getting fan speed control mode.
 *  @brief NetFn: 0x30, Cmd: 0x13
 *
 *  @return Byte 1: Completion Code.
 *          Byte 2: ControlMode - 00h Manual, 01h Auto.
 **/
ipmi_ret_t IpmiGetFscMode(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                          ipmi_request_t request, ipmi_response_t response,
                          ipmi_data_len_t dataLen, ipmi_context_t context)
{
    if (*dataLen != 0)
    {
        sd_journal_print(LOG_ERR, "[%s] invalid data length %d\n", __FUNCTION__,
                         *dataLen);
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    auto res = reinterpret_cast<GetFscModeResponse*>(response);

    // Check FSC mode is manual mode.
    if (std::filesystem::exists(manualModeFilePath) == true)
    {
        res->mode = FSC_MODE_MANUAL;
    }
    else
    {
        res->mode = FSC_MODE_AUTO;
    }

    *dataLen = sizeof(GetFscModeResponse);
    return IPMI_CC_OK;
}

static void register_oem_functions(void)
{
    // <Set Fan PWM>
    ipmi_register_callback(netFnc600g5OEM2, CMD_SET_FAN_PWM, NULL,
                           IpmiSetPwm, PRIVILEGE_USER);

    // <Set FSC Mode>
    ipmi_register_callback(netFnc600g5OEM2, CMD_SET_FSC_MODE, NULL,
                           IpmiSetFscMode, PRIVILEGE_USER);

    // <Get FSC Mode>
    ipmi_register_callback(netFnc600g5OEM2, CMD_GET_FSC_MODE, NULL,
                           IpmiGetFscMode, PRIVILEGE_USER);
}
