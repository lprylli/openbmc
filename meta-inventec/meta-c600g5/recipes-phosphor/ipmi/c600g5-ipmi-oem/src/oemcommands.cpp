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

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <variant>
#include <vector>

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

}

} // namespace ipmi
