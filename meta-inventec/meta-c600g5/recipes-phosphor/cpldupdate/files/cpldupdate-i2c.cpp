/*
 * cpldupdate-i2c.c - Update Lattice CPLD through I2c
 *
 * Copyright 2018-present Facebook. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include  <vector>
#include <sys/file.h>
#include <sys/stat.h>
#include <openbmc/obmc-i2c.h>
#include <sdbusplus/server.hpp>
#include "cpld_info.hpp"
#include <variant>
#include <tuple>
#include <iostream>

#ifdef DEBUG
#define CPLD_BEBUG(fmt, args...) printf(fmt, ##args);
#else
#define CPLD_BEBUG(fmt, args...)
#endif

#define ERR_PRINT(fmt, args...) \
        fprintf(stderr, fmt ": %s\n", ##args, strerror(errno));

#define BUSY_RETRIES (15)
#define UPDATE_RETRIES (3)
#define VERSION_ID_PATH_END (4)
#define RETRY_NUM (5)

const int VERIFY_PERCENTAGE = 40;
const int FLASH_PERCENTAGE = 40;

using GetSubTreeType = std::vector<std::pair<
                       std::string,
                       std::vector<std::pair<std::string, std::vector<std::string>>>>>;

auto OBJECT_MAPPER_SERVICE ="xyz.openbmc_project.ObjectMapper";
auto OBJECT_MAPPER_OBJECT ="/xyz/openbmc_project/object_mapper";
auto OBJECT_MAPPER_INTF= "xyz.openbmc_project.ObjectMapper";

auto PROP_INTF = "org.freedesktop.DBus.Properties";

auto ACTIVATION_PROGRESS_INTF = "xyz.openbmc_project.Software.ActivationProgress";
typedef struct _i2c_info_t {
    int fd;
    uint8_t bus;
    uint8_t addr;
} i2c_info_t;

enum {
    TRANSPARENT_MODE = 0x74,
    OFFLINE_MODE = 0xC6,
    CFG_PAGE = 0x46,
    UFM_PAGE = 0x47,
};

static void
print_usage(const char *name)
{
    printf("Usage: %s <bus> <address> <img_path>\n", name);
}

static int
i2c_open(uint8_t bus_num, uint8_t addr)
{
    int fd = -1, rc = -1;
    char fn[32];

    snprintf(fn, sizeof(fn), "/dev/i2c-%d", bus_num);
    fd = open(fn, O_RDWR);
    if (fd == -1) {
        ERR_PRINT("Failed to open i2c device %s", fn);
        return -1;
    }

    rc = ioctl(fd, I2C_SLAVE, addr);
    if (rc < 0) {
        ERR_PRINT("Failed to open slave @ address 0x%x", addr);
        close(fd);
        return -1;
    }
    return fd;
}

static int
ascii_to_hex(int ascii)
{

    ascii = ascii & 0xFF;
    if (ascii >= 0x30 && ascii <= 0x39) {
        return (ascii - 0x30);
    }/*0-9*/
    else if (ascii >= 0x41 && ascii <= 0x46) {
        return (ascii - 0x41 + 10);
    }/*A-F*/
    else if (ascii >= 0x61 && ascii <= 0x66) {
        return (ascii - 0x61 + 10);
    }/*a-f*/
    else {
        return -1;
    }
}

int i2c_rdwr_msg_transfer_retry(int file, __u8 addr, __u8 *tbuf,
                                __u8 tcount, __u8 *rbuf, __u8 rcount)
{
    int ret = -1;
    int count=0;

    for(count=0; count < RETRY_NUM; count++) {
        ret = i2c_rdwr_msg_transfer(file, addr, tbuf, tcount, rbuf, rcount);
        if(ret == 0) {
            break;
        }
        usleep(20000);
    }
    if (count >= RETRY_NUM) {
        printf("i2c retry fail!\n");
    }
    return ret;
}

static int
read_device_id(i2c_info_t cpld)
{

    uint8_t device_id_cmd[4] = {0xE0, 0x00, 0x00, 0x00};
    uint8_t device_id[4];
    int ret = -1;

    ret = i2c_rdwr_msg_transfer(cpld.fd, cpld.addr << 1, device_id_cmd,
                                sizeof(device_id_cmd), device_id, sizeof(device_id));
    if (ret != 0) {
        ERR_PRINT("read_device_id()");
        return ret;
    }
    CPLD_BEBUG("Read Device ID = 0x%X 0x%X 0x%X 0x%X -\n",
               device_id[0], device_id[1], device_id[2], device_id[3]);
    return 0;
}

static int
read_status(i2c_info_t cpld)
{
    uint8_t status_cmd[4] = {0x3C, 0x00, 0x00, 0x00};
    uint8_t status[4];
    int ret = -1;
    int i = 0;

    for(i = 0; i < RETRY_NUM; i++) {
        ret = i2c_rdwr_msg_transfer_retry(cpld.fd, cpld.addr << 1, status_cmd,
                                          sizeof(status_cmd), status, sizeof(status));
        if (ret != 0) {
            ERR_PRINT("read_busy_flag()");
            return ret;
	}

        if((status[2]&0x30) == 0) {
            break;
        }
        sleep(1);
    }

    if (i >= RETRY_NUM) {
        ERR_PRINT("read_status() is always busy");
        return ret;
    }
    CPLD_BEBUG("0x%X 0x%X 0x%X 0x%X Read Status\n",
               status[0],status[1],status[2],status[3]);
    return 0;
}

static int
read_busy_flag(i2c_info_t cpld)
{

    uint8_t busy_flag_cmd[4] = {0xF0, 0x00, 0x00, 0x00};
    uint8_t count = 0;
    uint8_t flag[1];
    int ret = -1;

    for (count = 0; count < BUSY_RETRIES; count++) {
        ret = i2c_rdwr_msg_transfer(cpld.fd, cpld.addr << 1, busy_flag_cmd,
                                    sizeof(busy_flag_cmd), flag, sizeof(flag));
        if (ret != 0) {
            ERR_PRINT("read_busy_flag()");
            return ret;
        }
        if (!(flag[0] & 0x80)) {
            return 0;
        }
        sleep(1);
    }

    return -1;
}

static int
enable_program_mode(i2c_info_t cpld, uint8_t mode)
{

    uint8_t enable_program_cmd[3] = {mode, 0x08, 0x00};
    int ret = -1;

    printf("Enable Program - ");
    if (mode == TRANSPARENT_MODE) {
        printf("Transparent Mode\n");
    } else if (mode == OFFLINE_MODE) {
        printf("Offline Mode\n");
    }
    ret = i2c_rdwr_msg_transfer(cpld.fd, cpld.addr << 1, enable_program_cmd,
                                sizeof(enable_program_cmd), NULL, 0);
    if (ret != 0) {
        ERR_PRINT("enable_program_mode()");
        return ret;
    }
    usleep(5);
    return 0;
}

static int
erase_flash(i2c_info_t cpld, uint8_t location)
{
    /* location[0]: SRAM
    * location[1]: FEATURE-ROW
    * location[2]: NVCM0/CFG
    * location[3]: NVCM1/UFM*/
    uint8_t erase_flash_cmd[4] = {0x0E, location, 0x00, 0x00};
    int ret = -1;

    printf("Erase Flash ");
    if (location & 0x1) {
        printf("SRAM ");
    }
    if (location & 0x2) {
        printf("FEATURE-ROW ");
    }
    if (location & 0x4) {
        printf("NVCM0/CFG ");
    }
    if (location & 0x8) {
        printf("NVCM1/UFM ");
    }
    printf("\n");

    ret = i2c_rdwr_msg_transfer(cpld.fd, cpld.addr << 1, erase_flash_cmd,
                                sizeof(erase_flash_cmd), NULL, 0);
    if (ret != 0) {
        ERR_PRINT("erase_flash()");
        return ret;
    }
    if (read_busy_flag(cpld) != 0) {
        CPLD_BEBUG("Device busy is caused by flash erase\n");
        return -1;
    }
    return 0;
}

static int
get_data_len(const char *file_path, int *cfg_len, int *ufm_len)
{

    int fd = -1;
    int i = 0, file_len = 0, ufm_start = 0;
    struct stat st;

    fd = open(file_path, O_RDONLY, 0666);
    if (fd < 0) {
        ERR_PRINT("get_data_len()");
        return -1;
    }

    stat(file_path, &st);
    file_len = st.st_size;
    CPLD_BEBUG("file_len: %d\n", file_len);

    uint8_t file_buf[file_len];

    if (read(fd, file_buf, file_len) < 0) {
        ERR_PRINT("get_data_len()");
    }

    for (i = 0; i < file_len; i+=34) {
        if (file_buf[i+32] == 0x0d && file_buf[i+33] == 0x0a) {
            *cfg_len = ((i + 34) / 34) * 32;
            if (file_buf[i+34] == 0x0d && file_buf[i+35] == 0x0a) {
                break;
            }
        } else {
            printf("get_data_len(): Invalid CFG content\n");
            return -1;
        }
    }
    *cfg_len = *cfg_len / 2;

    if (i >= file_len) {
        printf("get_data_len():Can't get seprator\n");
        return -1;
    }

    ufm_start = i + 34 + 2;
    for (i = ufm_start; i < file_len; i+=34) {
        if (file_buf[i+32] == 0x0d && file_buf[i+33] == 0x0a) {
            *ufm_len = ((i - ufm_start + 34) / 34) * 32;
        } else {
            printf("get_data_len(): Invalid UFM content\n");
            return -1;
        }
    }
    *ufm_len = *ufm_len / 2;
    return 0;
}

static int
get_img_data(const char *file_path, uint8_t cfg_data[], int cfg_len,
             uint8_t ufm_data[], int ufm_len)
{

    int fd = -1;
    int i = 0, j = 0, file_len= 0, ufm_start = 0;
    struct stat st;

    fd = open(file_path, O_RDONLY, 0666);
    if (fd < 0) {
        ERR_PRINT("get_img_data()");
        return -1;
    }

    stat(file_path, &st);
    file_len = st.st_size;

    uint8_t file_buf[file_len];

    if (read(fd, file_buf, file_len) < 0) {
        ERR_PRINT("get_img_data()");
        return -1;
    }

    for (i = 0, j = 0; i < cfg_len; i++, j+=2) {
        cfg_data[i] = (ascii_to_hex(file_buf[j])) << 4;
        cfg_data[i] |= ascii_to_hex(file_buf[j+1]);
        if (file_buf[j+2] == 0x0d && file_buf[j+3] == 0x0a) {
            j+=2;
        }
    }

    ufm_start = j + 2;
    for (i = 0 , j = ufm_start; i < ufm_len; i++, j+=2) {
        ufm_data[i] = (ascii_to_hex(file_buf[j])) << 4;
        ufm_data[i] |= ascii_to_hex(file_buf[j+1]);
        if (file_buf[j+2] == 0x0d && file_buf[j+3] == 0x0a) {
            j+=2;
        }
    }
    return 0;
}

/* This function is needed by Transparent Mode */
static int
refresh(i2c_info_t cpld)
{

    uint8_t refresh_cmd[3] = {0x79 ,0x00 ,0x00};
    int ret = -1;

    printf("Refreshing CPLD...");
    ret = i2c_rdwr_msg_transfer(cpld.fd, cpld.addr << 1, refresh_cmd,
                                sizeof(refresh_cmd), NULL, 0);
    if (ret != 0) {
        ERR_PRINT("\nrefresh()");
        return ret;
    }
    sleep(1);
    printf("Done\n");
    return 0;
}

static int
verify(i2c_info_t cpld, uint8_t page,
       uint8_t *data, int data_len, bool is_remote, const std::string& service, const std::string& object)
{

    uint8_t reset_addr_cmd[4] = {page, 0x00, 0x00, 0x00};
    /* 0x73 0x00: i2c, 0x73 0x10: JTAG/SSPI */
    uint8_t read_page_cmd[4] = {0x73, 0x00, 0x00, 0x01};
    uint8_t page_data[16] = {0};
    int byte_index = 0;
    int ret = -1;
    auto bus = sdbusplus::bus::new_default();
    std::variant<std::uint8_t> activation_progess;
    auto method=sdbusplus::message::message();
    auto reply=sdbusplus::message::message();
    uint8_t percentage_start = 0;
    uint8_t percentage = 0;

    /* Reset Page Address */
    ret = i2c_rdwr_msg_transfer(cpld.fd, cpld.addr << 1, reset_addr_cmd,
                                sizeof(reset_addr_cmd), NULL, 0);
    if (ret != 0) {
        ERR_PRINT("verify_flash(): Reset Page Address");
        return ret;
    }
    if (read_busy_flag(cpld) != 0) {
        CPLD_BEBUG("Device busy is caused by address reset\n");
        return -1;
    }

    if (page == CFG_PAGE) {
        printf("- Verify CFG Page -\n");
    } else if (page == UFM_PAGE) {
        printf("- Verify UFM Page -\n");
    }

    if( is_remote ) {
        method = bus.new_method_call(service.c_str(), object.c_str(), PROP_INTF, "Get");
        method.append(ACTIVATION_PROGRESS_INTF,"Progress");
        try {
            reply=bus.call(method);
            reply.read(activation_progess);
        } catch (const sdbusplus::exception::SdBusError& e) {
            printf("SdBusError!\n");
            return -1;
        }
        percentage_start = std::get<std::uint8_t>(activation_progess);
    }

    for (byte_index = 0; byte_index < data_len; byte_index+=16) {

        /* Read Page Data */
        ret = i2c_rdwr_msg_transfer(cpld.fd, cpld.addr << 1, read_page_cmd,
                                    sizeof(read_page_cmd), page_data, sizeof(page_data));
        if (ret != 0) {
            ERR_PRINT("verify_flash(): Read Page Data");
            return ret;
        }
        if (read_busy_flag(cpld) != 0) {
            CPLD_BEBUG("Device busy is caused by Read Page Data\n");
            return -1;
        }
        usleep(200);

        /* Compare Data */
        if (memcmp(page_data, data+byte_index, 16) != 0) {
            CPLD_BEBUG("\nImage_data: ");
            for (int i = 0; i < 16; i++) {
                CPLD_BEBUG("0x%2x ", page_data[i]);
            }
            CPLD_BEBUG("\nFlash_data: ");
            for (int i = 0; i < 16; i++) {
                CPLD_BEBUG("0x%2x ", data[byte_index+i]);
            }
            printf("\nCompare Fail - Do Clean Up Procedure\n");
            return -1;
        }
        printf("  (%d/%d) (%d%%/100%%)\r",
               byte_index + 16, data_len, (100 * (byte_index + 16) / data_len));
        if ((byte_index & 0xFFF) == 0) {
            if(is_remote) {
                percentage = percentage_start + (VERIFY_PERCENTAGE *( byte_index + 16)) / data_len;
                method = bus.new_method_call(service.c_str(), object.c_str() ,PROP_INTF, "Set");
                method.append(ACTIVATION_PROGRESS_INTF,"Progress");
                method.append(std::variant<uint8_t>(percentage));
                reply = bus.call(method);
                if (reply.is_method_error()) {
                    printf("error setting property: %s Progress",ACTIVATION_PROGRESS_INTF);
                    return -1;
                }
            }
        }
        usleep(200);
    }
    printf("...Done!\n");
    return 0;
}

static int
program_flash(i2c_info_t cpld, uint8_t page,
              uint8_t *data, int data_len, bool is_remote, const std::string& service, const std::string object)
{

    uint8_t reset_addr_cmd[4] = {page, 0x00, 0x00, 0x00};
    uint8_t write_page_cmd[4] = {0x70, 0x00, 0x00, 0x01};
    uint8_t program_page_cmd[32] = {0};
    int byte_index = 0;
    int ret = -1;
    auto bus = sdbusplus::bus::new_default();
    std::variant<std::uint8_t> activation_progess;
    auto method=sdbusplus::message::message() ;
    auto reply=sdbusplus::message::message() ;
    uint8_t percentage = 0;
    uint8_t percentage_start = 0;

    ret = i2c_rdwr_msg_transfer(cpld.fd, cpld.addr << 1, reset_addr_cmd,
                                sizeof(reset_addr_cmd), NULL, 0);
    if (ret != 0) {
        ERR_PRINT("program_flash(): Reset Page Address");
        return ret;
    }
    if (read_busy_flag(cpld) != 0) {
        CPLD_BEBUG("Device busy is caused by address reset.\n");
        return -1;
    }

    if (page == CFG_PAGE) {
        printf("- Program CFG Page -\n");
    } else if (page == UFM_PAGE) {
        printf("- Program UFM Page -\n");
    }

    if( is_remote ) {
        method = bus.new_method_call(service.c_str(), object.c_str(), PROP_INTF, "Get");
        method.append(ACTIVATION_PROGRESS_INTF,"Progress");
        try {
            reply=bus.call(method);
            reply.read(activation_progess);
        } catch (const sdbusplus::exception::SdBusError& e) {
            printf("SdBusError!\n");
            return -1;
        }
        percentage_start = std::get<std::uint8_t>(activation_progess);
    }

    memcpy(&program_page_cmd[0], write_page_cmd, 4);

    for (byte_index = 0; byte_index < data_len; byte_index += 16) {
        memcpy(&program_page_cmd[4], &data[byte_index], 16);
        CPLD_BEBUG("\n");
        for (int i = 0; i < 20; i++) {
            CPLD_BEBUG("0x%2x ", program_page_cmd[i]);
        }
        CPLD_BEBUG("\n");

        ret = i2c_rdwr_msg_transfer(cpld.fd, cpld.addr << 1, program_page_cmd,
                                    sizeof(program_page_cmd), NULL, 0);
        if (ret != 0) {
            ERR_PRINT("program_flash(): Program Page Data");
            return ret;
        }
        if (read_busy_flag(cpld) != 0) {
            CPLD_BEBUG("Device busy is caused by page data program.\n");
            return -1;
        }
        printf("  (%d/%d) (%d%%/100%%)\r",
               byte_index + 16, data_len, (100 * (byte_index + 16) / data_len));
        if ((byte_index & 0xFFF) == 0) {
            if(is_remote) {
                percentage =  percentage_start + (FLASH_PERCENTAGE * (byte_index + 16) / data_len);
                method = bus.new_method_call(service.c_str(), object.c_str() ,PROP_INTF, "Set");
                method.append(ACTIVATION_PROGRESS_INTF,"Progress");
                method.append(std::variant<uint8_t>(percentage));
                reply = bus.call(method);
                if (reply.is_method_error()) {
                    printf("error setting property: %s Progress",ACTIVATION_PROGRESS_INTF);
                    return -1;
                }
            }
        }
        usleep(200);
    }
    printf("\t\t\t\t...Done!\n");
    return 0;
}

static int
program_done(i2c_info_t cpld)
{
    uint8_t program_done_cmd[4] = {0x5E, 0x00, 0x00, 0x00};
    int ret = -1;

    printf("Program Done\n");
    ret = i2c_rdwr_msg_transfer(cpld.fd, cpld.addr << 1, program_done_cmd,
                                sizeof(program_done_cmd), NULL, 0);
    if (ret != 0) {
        ERR_PRINT("program_done()");
        return ret;
    }
    if (read_busy_flag(cpld) != 0) {
        CPLD_BEBUG("Device busy is caused by program done.\n");
        return -1;
    }
    return 0;
}

int
main(int argc, const char *argv[])
{
    bool is_remote = false;
    int cfg_len = 0, ufm_len = 0, pid_file = 0;
    i2c_info_t cpld;
    u_int8_t *cfg_data;
    u_int8_t *ufm_data;
    char *image_path;
    std::string object = "";
    std::string service = "";
    GetSubTreeType object_list;
    auto bus = sdbusplus::bus::new_default();
    auto reply =sdbusplus::message::message();
    int update_retry = 0;
    if (argc != 2) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    auto method = bus.new_method_call(OBJECT_MAPPER_SERVICE,OBJECT_MAPPER_OBJECT,OBJECT_MAPPER_INTF,"GetSubTree");
    method.append("/");
    method.append(0);

    std::vector<std::string> interface_list;
    interface_list.push_back(ACTIVATION_PROGRESS_INTF);

    method.append(interface_list);

    try {
        reply = bus.call(method);
        reply.read(object_list);
    } catch (const sdbusplus::exception::SdBusError& e) {
        printf("error: sdbus %s\n", e.what());
        return 0;
    }

    bus.release();
    bus.close();

    if(!object_list.empty()) {
        is_remote = true;
        object = object_list[0].first;
        service = object_list[0].second[0].first;
    }

    pid_file = open("/var/run/cpldupdate-i2c.pid", O_CREAT | O_RDWR, 0666);
    if(flock(pid_file, LOCK_EX | LOCK_NB) && (errno == EWOULDBLOCK)) {
        printf("Another cpldupdate-i2c instance is running...\n");
        exit(EXIT_FAILURE);
    }

    cpld.bus = CPLD_I2C_BUS;
    cpld.addr = CPLD_I2C_ADDR;

    cpld.fd = i2c_open(cpld.bus, cpld.addr);
    if (cpld.fd < 0) {
        return cpld.fd;
    }

    int count=0;
    int size = strlen(argv[1]) + 1;
    image_path=(char*)malloc(size);
    memcpy(image_path, argv[1], size); 
    
    if (is_remote) {
       /*when file path fed in by systemd service, need to replace '-' with '/' 
       until the slash  after version-id: ex: -tmp-images-123456-image_name */
       printf("filepath check\n", argv[1]);
       for(int i = 0; image_path[i] !='\0'; i++) {
	       if(image_path[i]=='-') {
	           image_path[i]='/';
	           count++;
	       }
	       if(count >= VERSION_ID_PATH_END) {
	           break;
	       }
        }
    }

    get_data_len(image_path, &cfg_len, &ufm_len);
    if (cfg_len <= 0 || ufm_len <= 0) {
        close(cpld.fd);
        return -1;
    }

    cfg_data = (uint8_t *) malloc(cfg_len);
    if (!cfg_data) {
        ERR_PRINT("CFG Data");
        close(cpld.fd);
        return ENOMEM;
    }

    ufm_data = (uint8_t *) malloc(ufm_len);
    if (!ufm_data) {
        ERR_PRINT("UFM Data");
        close(cpld.fd);
        return ENOMEM;
    }

    get_img_data(image_path, cfg_data, cfg_len, ufm_data, ufm_len);
    int rc = 0;
    if((rc = read_device_id(cpld)) !=0) {
        CPLD_BEBUG("failed to read device id\n");
        return -1;
    } else {
        CPLD_BEBUG("cpld update succeed\n");
    }

    if((rc = enable_program_mode(cpld, TRANSPARENT_MODE)) != 0) {
        CPLD_BEBUG("failed to enable program mode\n");
        return -1;
    } else {
        CPLD_BEBUG("enable program succeed\n");
    }

    for (update_retry = 0; update_retry < UPDATE_RETRIES; update_retry++) {

        sleep(1);
        if((rc = erase_flash(cpld, 0x04)) != 0) {
            CPLD_BEBUG("failed to erase flash\n");
            continue;
        } else {
            CPLD_BEBUG("erase flash succeed\n");
        }

        if (program_flash(cpld, CFG_PAGE, cfg_data, cfg_len, is_remote, service, object) != 0 ) {
            continue;
        }

        if (verify(cpld, CFG_PAGE, cfg_data, cfg_len, is_remote, service, object) != 0 ) {
            continue;
        } else {
            break;
        }
    }
    if (update_retry >= UPDATE_RETRIES) {

        printf("update retry fail\n");
        close(cpld.fd);
        free(cfg_data);
        free(ufm_data);
	free(image_path);
        return -1;
    }

    if((rc = program_done(cpld)) != 0) {
        printf("failed program done\n");
        close(cpld.fd);
        free(cfg_data);
        free(ufm_data);
	free(image_path);
        return -1;
    } else {
        printf("program done succeed\n");
    } 

    if(is_remote) {
        bus = sdbusplus::bus::new_default();
        method = bus.new_method_call(service.c_str(), object.c_str() ,PROP_INTF, "Set");
        method.append(ACTIVATION_PROGRESS_INTF,"Progress");
        uint8_t p = 100;
        method.append(std::variant<uint8_t>(p));
        try {
            reply = bus.call(method);
        } catch (const sdbusplus::exception::SdBusError& e) {
            printf("error: sdbus %s\n", e.what());
        }
    }

    if((rc = read_status(cpld)) != 0) {
        printf("failed read status\n");
        close(cpld.fd);
        free(cfg_data);
        free(ufm_data);
	free(image_path);
        return -1;
    } else {
        printf("read status succeed\n");
    }

    sleep(2);
    close(cpld.fd);
    free(cfg_data);
    free(ufm_data);
    free(image_path);
    printf("cpld update done, ready to refresh...\n");
    return 0;
}
