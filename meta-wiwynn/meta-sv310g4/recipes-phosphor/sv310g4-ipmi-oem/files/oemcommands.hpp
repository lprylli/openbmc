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

#pragma once

#include <ipmid/api.hpp>

#include <map>
#include <string>

static constexpr ipmi_netfn_t netFnSv310g4OEM1 = 0x2E;
static constexpr ipmi_netfn_t netFnSv310g4OEM2 = 0x30;
static constexpr ipmi_netfn_t netFnSv310g4OEM3 = 0x3E;
static constexpr ipmi_netfn_t netFnSv310g4OEM4 = 0x3C;

static const uint8_t oemMaxIPMIWriteReadSize = 50;

enum ipmi_sv310g4_oem3_cmds : uint8_t
{
    CMD_MASTER_WRITE_READ = 0x01,
};

struct MasterWriteReadReq
{
    uint8_t busId;
    uint8_t slaveAddr;
    uint8_t readCount;
    uint8_t writeData[oemMaxIPMIWriteReadSize];
};

struct MasterWriteReadRes
{
    uint8_t readResp[oemMaxIPMIWriteReadSize];
};
