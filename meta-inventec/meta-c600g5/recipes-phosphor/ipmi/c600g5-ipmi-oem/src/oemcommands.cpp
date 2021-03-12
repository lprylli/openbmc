/*
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include <oemcommands.hpp>
#include <commandutils.hpp>

#include <ipmid/api.hpp>
#include <ipmid/utils.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message/types.hpp>
#include <phosphor-logging/log.hpp>
#include <phosphor-logging/elog-errors.hpp>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <variant>
#include <vector>

#define PROPERTY_INTERFACE "org.freedesktop.DBus.Properties"
#include "xyz/openbmc_project/Control/Power/RestorePolicy/server.hpp"

using phosphor::logging::level;
using phosphor::logging::log;

using namespace phosphor::logging;
using namespace sdbusplus::xyz::openbmc_project::Control::Power::server;

namespace ipmi
{
static void registerOEMFunctions() __attribute__((constructor));

/*
An example of IPMI OEM command registration
*/
#if EXAMPLE
ipmi::RspType<bool, //just return the req param
             uint7_t // reserved
             >
    ipmiOemExampleCommand(bool req, uint7_t reserved1)
{
    return ipmi::responseSuccess(req, 0);
}
#endif

//===============================================================
/* Set/Get Ramdom Delay AC Restore Power ON Command
NetFun: 0x30
Cmd : 0x18
Request:
    Byte 1 : Op Code
        [7] : 0-Get 1-Set
        [6:0] :
            00h-Disable Delay
            01h-Enable Delay, Random Delay Time
            02h-Enable Delay, Fixed Delay Time
    Byte 2-3 : Delay Time, LSB first (Second base)
Response:
    Byte 1 : Completion Code
    Byte 2 :  Op Code
    Byte 3-4 : Delay Time, LSB first (Second base)
*/
ipmi::RspType<uint8_t, uint8_t, uint8_t> ipmiRamdomDelayACRestorePowerON(uint8_t opCode, uint8_t delayTimeLSB,uint8_t delayTimeMSB)
{
    std::uint8_t opCodeResponse, delayTimeLSBResponse, delayTimeMSBResponse;;

    constexpr auto service = "xyz.openbmc_project.Settings";
    constexpr auto path = "/xyz/openbmc_project/control/host0/power_restore_policy";
    constexpr auto powerRestoreInterface = "xyz.openbmc_project.Control.Power.RestorePolicy";
    constexpr auto alwaysOnPolicy = "PowerRestoreAlwaysOnPolicy";
    constexpr auto delay = "PowerRestoreDelay";

    auto bus = sdbusplus::bus::new_default();

    if(opCode & 0x80)
    {
        //Set
        opCodeResponse = opCode;
        delayTimeLSBResponse = delayTimeLSB;
        delayTimeMSBResponse = delayTimeMSB;
        try
        {
            auto method = bus.new_method_call(service, path, PROPERTY_INTERFACE,"Set");
            method.append(powerRestoreInterface, alwaysOnPolicy,std::variant<std::string>(RestorePolicy::convertAlwaysOnPolicyToString((RestorePolicy::AlwaysOnPolicy)(opCode & 0x7F))));
            bus.call_noreply(method);

            uint32_t delayValue = delayTimeLSB | (delayTimeMSB << 8);

            auto methodDelay = bus.new_method_call(service, path, PROPERTY_INTERFACE, "Set");
            methodDelay.append(powerRestoreInterface, delay,std::variant<uint32_t>(delayValue));
            bus.call_noreply(methodDelay);
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            log<level::ERR>("Error in RamdomDelayACRestorePowerON Set",entry("ERROR=%s", e.what()));
            return ipmi::responseParmOutOfRange();
        }
    }
    else
    {
        //Get
        auto method = bus.new_method_call(service, path, PROPERTY_INTERFACE,"Get");
        method.append(powerRestoreInterface, alwaysOnPolicy);

        std::variant<std::string> result;
        try
        {
            auto reply = bus.call(method);
            reply.read(result);
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            log<level::ERR>("Error in PowerRestoreAlwaysOnPolicy Get",entry("ERROR=%s", e.what()));
            return ipmi::responseUnspecifiedError();
        }
        auto powerAlwaysOnPolicy = std::get<std::string>(result);

        auto methodDelay = bus.new_method_call(service, path, PROPERTY_INTERFACE, "Get");
        methodDelay.append(powerRestoreInterface, delay);

        std::variant<uint32_t> resultDelay;
        try
        {
            auto reply = bus.call(methodDelay);
            reply.read(resultDelay);
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            log<level::ERR>("Error in PowerRestoreDelay Get",entry("ERROR=%s", e.what()));
            return ipmi::responseUnspecifiedError();
        }
        auto powerRestoreDelay = std::get<uint32_t>(resultDelay);

        opCodeResponse = (opCode & 0x80) | (uint8_t)RestorePolicy::convertAlwaysOnPolicyFromString(powerAlwaysOnPolicy);

        uint8_t *delayValue = (uint8_t *)&powerRestoreDelay;
        delayTimeLSBResponse = delayValue[0];
        delayTimeMSBResponse = delayValue[1];
    }

    return ipmi::responseSuccess(opCodeResponse,delayTimeLSBResponse,delayTimeMSBResponse);
}


/**
 *  Get Firmware Version
 *  Request:
 *          Target - 0x02 BMC
 *  Respond:
 *          ASCII encoded version
 **/
ipmi::RspType<std::vector<uint8_t>> ipmiGetFirmwareVersion(uint8_t target)
{
    std::vector<uint8_t> requestedData;
    char tmp_buffer[128] = {0};
    char* keyword_ptr;
    int8_t ver_len = 0;
    std::ifstream file_read;

    switch (FwVersionTarget(target))
    {
        case FwVersionTarget::FW_BMC:
        {
            if constexpr (debug)
            {
                std::fprintf(stderr, "[ipmiGetFirmwareVersion] FW_BMC\n");
            }

            file_read.open("/etc/os-release", std::ifstream::in);
            if (file_read.is_open())
            {
                while (file_read.getline(tmp_buffer, 128))
                {
                    keyword_ptr = strstr(tmp_buffer, "VERSION=");
                    if (keyword_ptr != NULL)
                    {
                        break;
                    }
                }
            }

            file_read.close();

            if (keyword_ptr == NULL)
            {
                return ipmi::responseUnspecifiedError();
            }

            try
            {
                keyword_ptr = strchr(tmp_buffer, '"');
                keyword_ptr = strtok(keyword_ptr, "\"");
                if constexpr (debug)
                {
                    std::fprintf(stderr, "[ipmiGetFirmwareVersion] Get Value: %s\n",
                                         keyword_ptr);
                }

                if (keyword_ptr != NULL)
                {
                    ver_len = strlen(keyword_ptr); // report the name length
                    strcpy(tmp_buffer, keyword_ptr);
                    requestedData.insert(requestedData.begin(),
                                         tmp_buffer,
                                         tmp_buffer + ver_len
                                        );
                }
            }
            catch (std::exception& e)
            {
                std::fprintf(stderr, "[ipmiGetFirmwareVersion] Get BMC Ver fail\n");
                return ipmi::responseUnspecifiedError();
            }

            break;
        }
        default:
        {
            if constexpr (debug)
            {
                std::fprintf(stderr, "[ipmiGetFirmwareVersion] FW Unknow\n");
            }
            return ipmi::responseParmOutOfRange();
            break;
        }
    }

    return ipmi::responseSuccess(requestedData);
}

static void registerOEMFunctions(void)
{
    phosphor::logging::log<phosphor::logging::level::INFO>(
        "Registering C600G5 OEM commands");

    /*This is an example of IPMI OEM command registration*/
#if EXAMPLE
    registerOemCmdHandler(c600g5::netFnOem_3E, c600g5::netFn_3E_cmds::cmdExample,
                            Privilege::Admin, ipmiOemExampleCommand);
#endif

    // <Get Firmware Version>
    registerOemCmdHandler(c600g5::netFnOem_38, c600g5::netFn_38_cmds::cmdGetFwVersion,
                            Privilege::User, ipmiGetFirmwareVersion);

    // <Set/Get Random delay>
    registerOemCmdHandler(c600g5::netFnOem_30, c600g5::netFn_30_cmds::cmdRamdomDelayACRestorePowerON,
                            Privilege::User, ipmiRamdomDelayACRestorePowerON);
}

} // namespace ipmi
