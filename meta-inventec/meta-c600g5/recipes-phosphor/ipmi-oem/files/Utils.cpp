
#include "Utils.hpp"

#include <time.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <regex>

vector<string> getDirFiles(string dirPath, string regexStr)
{
    vector<string> result;

    for (const auto& entry : filesystem::directory_iterator(dirPath))
    {
        // If filename matched the regular expression put it in result.
        if (regex_match(entry.path().filename().string(), regex(regexStr)))
        {
            result.emplace_back(move(entry.path().string()));
        }
    }

    return result;
}

DbusSubTree getSubTree(sdbusplus::bus::bus& bus, const std::string& pathRoot,
                       int depth, const std::string& intf)
{
    DbusSubTree subTree;
    auto subTreeMsg =
        bus.new_method_call("xyz.openbmc_project.ObjectMapper",
                            "/xyz/openbmc_project/object_mapper",
                            "xyz.openbmc_project.ObjectMapper", "GetSubTree");

    const std::vector<std::string> interfaces = {intf};

    subTreeMsg.append(pathRoot, depth, interfaces);

    sdbusplus::message::message reply;
    try
    {
        reply = bus.call(subTreeMsg);
    }
    catch (sdbusplus::exception::SdBusError& e)
    {
        sd_journal_print(LOG_ERR, "GetSubTree Failed in bus call, %s\n",
                         e.what());
    }

    try
    {
        reply.read(subTree);
    }
    catch (sdbusplus::exception::SdBusError& e)
    {
        sd_journal_print(LOG_ERR, "GetSubTree Failed in read reply, %s\n",
                         e.what());
    }

    return subTree;
}

bool getDimmConfig(std::vector<uint8_t>& dimmConfig, const std::string& path)
{
    if (dimmConfig.empty() == false)
    {
        return true;
    }

    std::ifstream dimmConfigFile(path);
    if (dimmConfigFile.is_open() == false)
    {
        sd_journal_print(LOG_ERR, "[%s] Failed to open DIMM config file: %s\n",
                         __FUNCTION__, path.c_str());
        return false;
    }

    auto data = nlohmann::json::parse(dimmConfigFile, nullptr, false);
    if (data.is_discarded())
    {
        dimmConfigFile.close();
        sd_journal_print(LOG_ERR, "[%s] Syntax error in %s\n", __FUNCTION__,
                         path.c_str());
        return false;
    }

    for (const auto& config : data)
    {
        auto findNumber = config.find("Number");
        auto findName = config.find("Name");
        if (findNumber == config.end() || findName == config.end())
        {
            dimmConfig.clear();
            dimmConfigFile.close();
            sd_journal_print(LOG_ERR, "[%s] Invalid DIMM configuration data\n",
                             __FUNCTION__);
            return false;
        }

        dimmConfig.push_back(static_cast<uint8_t>(*findNumber));
    }

    dimmConfigFile.close();
    return true;
}

static constexpr auto GPIO_ROOT_PATH = "/sys/class/gpio/gpio";
static constexpr auto GPIO_EXPORT_PATH = "/sys/class/gpio/export";
static constexpr auto GPIO_UNEXPORT_PATH = "/sys/class/gpio/unexport";
static constexpr auto GPIO_DIRECTION_OUT_STR = "out";
static constexpr auto GPIO_DIRECTION_IN_STR = "in";

static constexpr size_t MAX_RETRY = 5;

// Add non-busy-wait msleep() function
void msleep(int32_t msec)
{
    struct timespec req;

    req.tv_sec = 0;
    req.tv_nsec = msec * 1000 * 1000;

    while (nanosleep(&req, &req) == -1 && errno == EINTR)
    {
        continue;
    }
}

int exportGPIO(int gpioNum)
{
    auto gpioPath = GPIO_ROOT_PATH + std::to_string(gpioNum);

    if (std::filesystem::exists(gpioPath) == false)
    {
        int retry = MAX_RETRY;
        while (retry--)
        {
            std::ofstream fileExport(GPIO_EXPORT_PATH);
            if (fileExport.is_open() == false)
            {
                msleep(10);
                continue;
            }

            fileExport << gpioNum;
            if (fileExport.fail())
            {
                fileExport.close();
                msleep(10);
                continue;
            }

            fileExport.close();
            break;
        }

        if (retry == 0)
        {
            sd_journal_print(LOG_ERR, "[%s] failed to export device %s\n",
                             __FUNCTION__, gpioPath);
            return -1;
        }
    }

    return 0;
}

int unexportGPIO(int gpioNum)
{
    auto gpioPath = GPIO_ROOT_PATH + std::to_string(gpioNum);

    if (std::filesystem::exists(gpioPath) == true)
    {
        int retry = MAX_RETRY;
        while (retry--)
        {
            std::ofstream fileExport(GPIO_UNEXPORT_PATH);
            if (fileExport.is_open() == false)
            {
                msleep(10);
                continue;
            }

            fileExport << gpioNum;
            if (fileExport.fail())
            {
                fileExport.close();
                msleep(10);
                continue;
            }

            fileExport.close();
            break;
        }

        if (retry == 0)
        {
            sd_journal_print(LOG_ERR, "[%s] failed to unexport device %s\n",
                             __FUNCTION__, gpioPath);
            return -1;
        }
    }

    return 0;
}

int getGPIOValue(int gpioNum, uint8_t& value)
{
    auto gpioValuePath = GPIO_ROOT_PATH + std::to_string(gpioNum) + "/value";

    if (std::filesystem::exists(gpioValuePath) == true)
    {
        int retry = MAX_RETRY;
        while (retry--)
        {
            std::ifstream fileValue(gpioValuePath);
            if (fileValue.is_open() == false)
            {
                msleep(10);
                continue;
            }

            int valueInt;
            fileValue >> valueInt;
            if (fileValue.fail())
            {
                fileValue.close();
                msleep(10);
                continue;
            }
            value = valueInt;

            fileValue.close();
            break;
        }

        if (retry == 0)
        {
            sd_journal_print(LOG_ERR, "[%s] failed to get value %s\n",
                             __FUNCTION__, gpioValuePath);
            return -1;
        }
    }
    else
    {
        sd_journal_print(LOG_ERR, "[%s] gpio not export yet %s\n", __FUNCTION__,
                         gpioValuePath);
        return -1;
    }

    return 0;
}

int getGPIODirection(int gpioNum, uint8_t& direction)
{
    auto gpioDirectionPath =
        GPIO_ROOT_PATH + std::to_string(gpioNum) + "/direction";

    if (std::filesystem::exists(gpioDirectionPath) == true)
    {
        int retry = MAX_RETRY;
        while (retry--)
        {
            std::ifstream fileDirection(gpioDirectionPath);
            if (fileDirection.is_open() == false)
            {
                msleep(10);
                continue;
            }

            std::string directionStr;
            fileDirection >> directionStr;
            if (fileDirection.fail())
            {
                fileDirection.close();
                msleep(10);
                continue;
            }
            fileDirection.close();

            if (directionStr == GPIO_DIRECTION_IN_STR)
            {
                direction = GPIO_DIRECTION_IN;
            }
            else if (directionStr == GPIO_DIRECTION_OUT_STR)
            {
                direction = GPIO_DIRECTION_OUT;
            }
            else
            {
                return -1;
            }

            break;
        }

        if (retry == 0)
        {
            sd_journal_print(LOG_ERR, "[%s] failed to get direction %s\n",
                             __FUNCTION__, gpioDirectionPath);
            return -1;
        }
    }
    else
    {
        sd_journal_print(LOG_ERR, "[%s] gpio not export yet %s\n", __FUNCTION__,
                         gpioDirectionPath);
        return -1;
    }

    return 0;
}

int setGPIOValue(int gpioNum, uint8_t value)
{
    if (value != GPIO_VALUE_LOW && value != GPIO_VALUE_HIGH)
    {
        return -1;
    }

    auto gpioValuePath = GPIO_ROOT_PATH + std::to_string(gpioNum) + "/value";

    if (std::filesystem::exists(gpioValuePath) == true)
    {
        int retry = MAX_RETRY;
        while (retry--)
        {
            std::ofstream fileValue(gpioValuePath);
            if (fileValue.is_open() == false)
            {
                msleep(10);
                continue;
            }

            fileValue << static_cast<int>(value);
            if (fileValue.fail())
            {
                fileValue.close();
                msleep(10);
                continue;
            }

            fileValue.close();
            break;
        }

        if (retry == 0)
        {
            sd_journal_print(LOG_ERR, "[%s] failed to set value %s\n",
                             __FUNCTION__, gpioValuePath);
            return -1;
        }
    }
    else
    {
        sd_journal_print(LOG_ERR, "[%s] gpio not export yet %s\n", __FUNCTION__,
                         gpioValuePath);
        return -1;
    }

    return 0;
}

int setGPIODirection(int gpioNum, uint8_t direction)
{
    std::string directionStr;
    if (direction == GPIO_DIRECTION_IN)
    {
        directionStr = GPIO_DIRECTION_IN_STR;
    }
    else if (direction == GPIO_DIRECTION_OUT)
    {
        directionStr = GPIO_DIRECTION_OUT_STR;
    }
    else
    {
        return -1;
    }

    auto gpioDirectionPath =
        GPIO_ROOT_PATH + std::to_string(gpioNum) + "/direction";

    if (std::filesystem::exists(gpioDirectionPath) == true)
    {
        int retry = MAX_RETRY;
        while (retry--)
        {
            std::ofstream fileDirection(gpioDirectionPath);
            if (fileDirection.is_open() == false)
            {
                msleep(10);
                continue;
            }

            fileDirection << directionStr;
            if (fileDirection.fail())
            {
                fileDirection.close();
                msleep(10);
                continue;
            }
            fileDirection.close();
            break;
        }

        if (retry == 0)
        {
            sd_journal_print(LOG_ERR, "[%s] failed to set direction %s\n",
                             __FUNCTION__, gpioDirectionPath);
            return -1;
        }
    }
    else
    {
        sd_journal_print(LOG_ERR, "[%s] gpio not export yet %s\n", __FUNCTION__,
                         gpioDirectionPath);
        return -1;
    }

    return 0;
}
