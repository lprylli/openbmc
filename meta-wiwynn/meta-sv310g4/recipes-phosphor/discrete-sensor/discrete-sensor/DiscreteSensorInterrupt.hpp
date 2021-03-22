#pragma once

#include <openbmc/libobmcdbus.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/server.hpp>

#include <functional>
#include <iostream>
#include <map>
#include <string>

constexpr auto discreteServ = "xyz.openbmc_project.Discrete.Sensor";
constexpr auto discretePathPrefix = "/xyz/openbmc_project/sensors/discrete/";
constexpr auto discreteIntf = "xyz.openbmc_project.Discrete.Sensor";
constexpr auto valueIntf = "xyz.openbmc_project.Sensor.Value";
constexpr auto maxAddSelRetryTime = 3;

std::map<
    std::string,
    std::map<std::string, std::shared_ptr<sdbusplus::asio::dbus_interface>>>
    interfaces;

int addSelWithRetry(std::string message, std::string objPath,
                    std::vector<uint8_t> eventData, bool is_assert,
                    uint8_t retryTime)
{
    if (retryTime >= maxAddSelRetryTime)
    {
        return -1;
    }
    else
    {
        if (ipmiSelAdd(message, objPath, eventData, is_assert) != 0)
        {
            return addSelWithRetry(message, objPath, eventData, is_assert,
                                   (retryTime + 1));
        }
    }

    return 0;
}
