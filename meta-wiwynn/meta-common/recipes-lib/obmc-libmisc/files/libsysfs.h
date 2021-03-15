#ifndef _LIBSYSFS_H
#define _LIBSYSFS_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>
#include <stdlib.h> 

#define SEM_PMBUS "/pmbus"
#define SEM_BIOSUPDATELOCK "/bios_update_lock"
#define SEM_PSUUPDATELOCK "/psu_update_lock"
#define SEM_TIMEOUT_S  2
#define SEM_TIMEOUT_NS 0
#define SEM_MAX_NUM 1
#define SEM_MAX_RETRY 3

int32_t test = 32;

int32_t Sem_Acquired(const char *name);
int32_t Sem_Released(const char *name);

#ifdef  __cplusplus
}
#endif

#endif /* _LIBSYSFS_H */