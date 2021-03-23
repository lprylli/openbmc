#include "DiscreteSensorInterrupt.hpp"
#include "Sensor.hpp"

#include <openbmc/libmisc.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/container/flat_set.hpp>
#include <nlohmann/json.hpp>
#include <sdbus_asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/exception.hpp>
#include <sdbusplus/message/types.hpp>
#include <sdbusplus/server.hpp>

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <variant>
#include <filesystem>

extern "C"
{
#include <gpiod.h>
}
using json = nlohmann::json;

std::vector<std::shared_ptr<sdbusplus::asio::connection>> buses;
extern std::map<
    std::string,
    std::map<std::string, std::shared_ptr<sdbusplus::asio::dbus_interface>>>
    interfaces;

const std::array<const char*, 2> entityIntf = {
    "xyz.openbmc_project.Inventory.Item.Board",
    "xyz.openbmc_project.Inventory.Item.Chassis"};

json loadJson(const std::string& path)
{
    std::ifstream jsonFile(path);
    if (jsonFile.is_open() == false)
    {
        throw std::runtime_error("Failed to open json file");
    }

    auto data = json::parse(jsonFile, nullptr, false);
    if (data.is_discarded())
    {
        throw std::runtime_error("Failed to parse json - invalid format");
    }

    return data;
}

void registerProperty(
    std::string& name, std::string& signature, json& value,
    std::shared_ptr<sdbusplus::asio::dbus_interface> interface)
{
    if ((signature == "y") || (signature == "UINT8"))
    {
        interface->register_property(
            name, (uint8_t)value,
            sdbusplus::asio::PropertyPermission::readWrite);
    }
    else if ((signature == "b") || (signature == "BOOL"))
    {
        interface->register_property(
            name, (bool)value, sdbusplus::asio::PropertyPermission::readWrite);
    }
    else if ((signature == "n") || (signature == "INT16"))
    {
        interface->register_property(
            name, (int16_t)value,
            sdbusplus::asio::PropertyPermission::readWrite);
    }
    else if ((signature == "q") || (signature == "UINT16"))
    {
        interface->register_property(
            name, (uint16_t)value,
            sdbusplus::asio::PropertyPermission::readWrite);
    }
    else if ((signature == "i") || (signature == "INT32"))
    {
        interface->register_property(
            name, (int32_t)value,
            sdbusplus::asio::PropertyPermission::readWrite);
    }
    else if ((signature == "u") || (signature == "UINT32"))
    {
        interface->register_property(
            name, (uint32_t)value,
            sdbusplus::asio::PropertyPermission::readWrite);
    }
    else if ((signature == "x") || (signature == "INT64"))
    {
        interface->register_property(
            name, (int64_t)value,
            sdbusplus::asio::PropertyPermission::readWrite);
    }
    else if ((signature == "t") || (signature == "UINT64"))
    {
        interface->register_property(
            name, (uint64_t)value,
            sdbusplus::asio::PropertyPermission::readWrite);
    }
    else if ((signature == "d") || (signature == "DOUBLE"))
    {
        interface->register_property(
            name, (double)value,
            sdbusplus::asio::PropertyPermission::readWrite);
    }
    else if ((signature == "s") || (signature == "STRING"))
    {
        interface->register_property(
            name, (std::string)value,
            sdbusplus::asio::PropertyPermission::readWrite);
    }
    else
    {
        std::cerr << "Unknown Signature: " << signature << "\n";
    }

    return;
}

int main(int argc, char** argv)
{
    json jsonData;
    try
    {
        jsonData = loadJson(std::string(DISCRETE_SENSOR_DBUS_PATH));
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to load json file: " << e.what() << "\n";
        return 0;
    }
    // Async io context for operation.
    auto io = std::make_shared<boost::asio::io_context>();
    setIoContext(io);
    auto conn = std::make_shared<sdbusplus::asio::connection>(*io);
    setSdBus(conn); // Register Dbus as discreate sensor
    conn->request_name(discreteServ);
    auto server = sdbusplus::asio::object_server(conn);

    for (auto& obj : jsonData)
    {
        auto objNameFind = obj.find("ObjPathName");
        auto objInfoFind = obj.find("ObjPathInfo");
        if ((objNameFind != obj.end()) && (objInfoFind != obj.end()))
        {
            std::string objPathName = *objNameFind;

            for (auto& intf : *objInfoFind)
            {
                auto intfNameFind = intf.find("InterfaceName");
                auto intfInfoFind = intf.find("InterfaceInfo");
                if ((intfNameFind != intf.end()) &&
                    (intfInfoFind != intf.end()))
                {
                    std::string intfName = *intfNameFind;
                    auto iface = server.add_interface(objPathName, intfName);
                    interfaces[objPathName][intfName] = iface;

                    for (auto& prop : *intfInfoFind)
                    {
                        auto propNameFind = prop.find("Name");
                        auto propTypeFind = prop.find("Type");
                        auto propSignatureFind = prop.find("Signature");
                        auto propMethodFind = prop.find("Function");
                        std::string name;
                        std::string type;
                        if ((propNameFind != prop.end()) &&
                            (propTypeFind != prop.end()))
                        {
                            name = *propNameFind;
                            type = *propTypeFind;
                        }
                        else
                        {
                            continue;
                        }

                        if (type == "property")
                        {
                            auto propChipIdFind = prop.find("ChipId");
                            auto propGpioNumFind = prop.find("GpioNum");
                            if (propSignatureFind != prop.end())
                            {
                                std::string signature = *propSignatureFind;

                                if ((propChipIdFind != prop.end()) &&
                                    (propGpioNumFind != prop.end()))
                                {
                                    std::string chipId = *propChipIdFind;
                                    int gpioNum = *propGpioNumFind;
                                    int gpioValue = gpiod_ctxless_get_value(
                                        chipId.c_str(), gpioNum, false,
                                        argv[0]);
                                    if (gpioValue < 0)
                                    {
                                        std::cerr
                                            << "Failed to get gpio value. "
                                               "chipId: "
                                            << chipId << " gpioNum: " << gpioNum
                                            << " return: " << gpioValue << "\n";
                                        continue;
                                    }
                                    prop["Value"] = gpioValue;
                                }
                                else
                                {
                                    auto valueFind = prop.find("Value");
                                    double value = 0;
                                    if (valueFind == prop.end())
                                    {
                                        prop["Value"] = value;
                                    }
                                }

                                registerProperty(name, signature,
                                                 prop.at("Value"), iface);
                            }
                        }
                    }
                    iface->initialize();
                }
            }
            auto objSensorTypeFind = obj.find("SensorType");
            if (objSensorTypeFind != obj.end())
            {
                std::string sensorType = *objSensorTypeFind;
                if (sensorType == "Discrete")
                {
                    sdbusplus::message::message msg = conn->new_method_call(
                        "xyz.openbmc_project.ObjectMapper",
                        "/xyz/openbmc_project/object_mapper",
                        "xyz.openbmc_project.ObjectMapper", "GetSubTreePaths");
                    msg.append("/xyz/openbmc_project/inventory", 0, entityIntf);
                    try
                    {
                        auto reply = conn->call(msg);
                        std::vector<std::string> propValue;
                        reply.read(propValue);
                        for (auto boardPath : propValue)
                        {
                            std::string assoIntf =
                                "xyz.openbmc_project.Association.Definitions";
                            auto iface =
                                server.add_interface(objPathName, assoIntf);
                            std::vector<std::tuple<std::string, std::string,
                                                   std::string>>
                                associations;
                            associations.push_back(
                                std::tuple<std::string, std::string,
                                           std::string>(
                                    "chassis", "all_sensors", boardPath));
                            iface->register_property("Associations",
                                                     associations);
                            iface->initialize();
                        }
                    }
                    catch (sdbusplus::exception_t& e)
                    {
                        std::cerr << "call GetSubTreePaths failed\n";
                    }
                }
            }
            auto objMonitorTypeFind = obj.find("MonitorType");
            if (objMonitorTypeFind != obj.end())
            {
                std::string type = *objMonitorTypeFind;
                if (type == "OneShot")
                {
                    std::string sensor_name;
                    std::size_t found_sensor_name =
                        objPathName.find_last_of("/\\");
                    sensor_name = objPathName.substr(found_sensor_name + 1);
                    if (sensor_name == "BMC_Reboot")
                    {
                        const uint32_t wdt1StatusReg = 0x1e785010;
                        const uint32_t wdtOffset = 0x20;
                        const uint32_t sRAMReg = 0x1e723000;
                        const uint32_t wdt1ClearStatusReg = 0x1e785014;
                        const uint32_t clearRegValue = 0x76;

                        uint32_t wdt1RegValue = 0;
                        uint32_t wdt2RegValue = 0;
                        uint32_t wdt3RegValue = 0;

                        if (read_register(wdt1StatusReg, &wdt1RegValue) < 0)
                        {
                            std::cerr << "Failed to read register WDT1 \n";
                            continue;
                        }

                        if (read_register(wdt1StatusReg + wdtOffset,
                                          &wdt2RegValue) < 0)
                        {
                            std::cerr << "Failed to read register WDT2 \n";
                            continue;
                        }

                        if (read_register(wdt1StatusReg + 2 * wdtOffset,
                                          &wdt3RegValue) < 0)
                        {
                            std::cerr << "Failed to read register WDT3 \n";
                            continue;
                        }

                        if (((wdt1RegValue >> 3) != 0) ||
                            ((wdt2RegValue >> 3) != 0) ||
                            ((wdt3RegValue >> 3) != 0))
                        {
                            // ASCII pani
                            const uint32_t panicRegValue1 = 0x50414E49;
                            // ASCII c & checksum
                            const uint32_t panicRegValue2 = 0x43950000;

                            uint32_t sRAM1RegValue = 0;
                            uint32_t sRAM2RegValue = 0;
                            if (read_register(sRAMReg, &sRAM1RegValue) < 0)
                            {
                                std::cerr << "Failed to read register SRAM1\n";
                            }
                            if (read_register(sRAMReg + 4, &sRAM2RegValue) < 0)
                            {
                                std::cerr << "Failed to read register SRAM2\n";
                            }

                            std::string message;
                            std::vector<uint8_t> eventData(3, 0xFF);

                            // Reboot by kernel panic
                            if (sRAM1RegValue == panicRegValue1 &&
                                sRAM2RegValue == panicRegValue2)
                            {
                                message = "Kernel Panic";
                                // Controller access degraded or unavailable.
                                eventData[0] = 0x01;
                            }
                            else
                            {
                                message = "BMC Reboot";
                                // Management controller off-line
                                eventData[0] = 0x02;
                            }

                            ipmiSelAdd(message, objPathName, eventData, true);
                        }

                        // Reset all register
                        write_register(wdt1ClearStatusReg, clearRegValue);
                        write_register(wdt1ClearStatusReg + wdtOffset,
                                       clearRegValue);
                        write_register(wdt1ClearStatusReg + 2 * wdtOffset,
                                       clearRegValue);
                        write_register(sRAMReg, 0x0);
                        write_register(sRAMReg + 4, 0x0);
                    }
                    else if (sensor_name == "Power_Unit")
                    {
                        /* AC lost */
                        std::filesystem::path filePath("/run/openbmc/AC-lost@0");
                        if(std::filesystem::exists(filePath))
                        {
                            std::string message;
                            std::vector<uint8_t> eventData(3, 0xFF);
                            message = "AC lost";
                            eventData.at(0) = 0x04;
                            ipmiSelAdd(message, objPathName, eventData, true);
                        }
                    }
                }
            }
        }
    }

    io->run();

    return 0;
}
