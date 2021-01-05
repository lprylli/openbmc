/*
// Copyright (c) 2020 Wiwynn Corporation
//
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

/*
    OEM implements master write read IPMI command
    request :
    byte 1 : Bus ID
    byte 2 : Slave address (8 bits)
    byte 3 : Read count
    byte 4 - n : WriteData

    response :
    byte 1 : CC code
    byte 2 - n : Response data (Maximum = 50 bytes)
 */

ipmi_ret_t ipmiMasterWriteRead(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                               ipmi_request_t request, ipmi_response_t response,
                               ipmi_data_len_t dataLen, ipmi_context_t context)
{
    auto req = reinterpret_cast<MasterWriteReadReq*>(request);
    auto res = reinterpret_cast<MasterWriteReadRes*>(response);

    int ret = -1;
    int fd = -1;
    // Shift right 1 bit, use 7 bits data as slave address.
    uint8_t oemSlaveAddr = req->slaveAddr >> 1;
    // Write data length were counted from the fourth byte of request command.
    const size_t writeCount = *dataLen - 3;
    std::vector<char> filename;
    filename.assign(32, 0);

    fd = open_i2c_dev(req->busId, filename.data(), filename.size(), 0);
    if (fd < 0)
    {
        sd_journal_print(LOG_CRIT, "Fail to open I2C device:[%d]\n", __LINE__);
        return IPMI_CC_BUSY;
    }

    if (req->readCount > oemMaxIPMIWriteReadSize)
    {
        sd_journal_print(LOG_ERR, "Master write read command: "
                                  "Read count exceeds limit:%d bytes\n", oemMaxIPMIWriteReadSize);
        close_i2c_dev(fd);
        return  IPMI_CC_PARM_OUT_OF_RANGE;
    }

    if (!req->readCount && !writeCount)
    {
        sd_journal_print(LOG_ERR, "Master write read command: "
                                  "Read & write count are 0");
        close_i2c_dev(fd);
        return  IPMI_CC_INVALID_FIELD_REQUEST;
    }

    if(req->readCount == 0)
    {
        ret = i2c_master_write(fd, oemSlaveAddr, writeCount, req->writeData);
    }
    else
    {
        ret = i2c_master_write_read(fd, oemSlaveAddr, writeCount, req->writeData,
                                    req->readCount, res->readResp);
    }

    if (ret < 0)
    {
        sd_journal_print(LOG_CRIT, "i2c_master_write_read failed: OEM master write read\n");
        close_i2c_dev(fd);
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    close_i2c_dev(fd);

    *dataLen = req->readCount;

    return IPMI_CC_OK;
}

static void register_oem_functions(void)
{
    // <Master Write Read>
    ipmi_register_callback(netFnSv310g4OEM3, CMD_MASTER_WRITE_READ,
                           NULL, ipmiMasterWriteRead, PRIVILEGE_USER);
}
