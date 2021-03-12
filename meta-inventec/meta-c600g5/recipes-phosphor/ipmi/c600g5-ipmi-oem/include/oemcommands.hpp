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

#pragma once

#include <ipmid/api-types.hpp>
#include <stdexcept>

#define EXAMPLE 0 

enum class ServiceType : uint8_t
{
    Service_Web = 0x01,
};

enum class FwVersionTarget : uint8_t
{
    FW_BMC = 0x02,
};

namespace ipmi
{
namespace c600g5
{

static constexpr NetFn netFnOem_30 = netFnOemOne;
static constexpr NetFn netFnOem_32 = netFnOemTwo;
static constexpr NetFn netFnOem_34 = netFnOemThree;
static constexpr NetFn netFnOem_36 = netFnOemFour;
static constexpr NetFn netFnOem_38 = netFnOemFive;
static constexpr NetFn netFnOem_3A = netFnOemSix;
static constexpr NetFn netFnOem_3C = netFnOemSeven;
static constexpr NetFn netFnOem_3E = netFnOemEight;

namespace netFn_30_cmds
{
    static constexpr Cmd cmdRamdomDelayACRestorePowerON = 0x18;
    static constexpr Cmd cmdSetServiceStatus = 0x0D;
    static constexpr Cmd cmdGetServiceStatus = 0x0E;
} // namespace netFn_30_cmds

namespace netFn_32_cmds
{
} // namespace netFn_32_cmds

namespace netFn_34_cmds
{
} // namespace netFn_34_cmds

namespace netFn_36_cmds
{
} // namespace netFn_36_cmds

namespace netFn_38_cmds
{
    static constexpr Cmd cmdGetFwVersion = 0x0B;
} // namespace netFn_38_cmds

namespace netFn_3A_cmds
{
} // namespace netFn_3A_cmds

namespace netFn_3C_cmds
{
} // namespace netFn_3C_cmds

namespace netFn_3E_cmds
{
//An example of IPMI OEM command registration
#if EXAMPLE
static constexpr Cmd cmdExample = 0xff;
#endif

} // namespace netFn_3E_cmds



} // namespace c600g5
} // namespace ipmi
