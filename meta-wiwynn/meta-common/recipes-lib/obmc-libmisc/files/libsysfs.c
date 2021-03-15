#include "libsysfs.h"

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <systemd/sd-journal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>

int32_t Sem_Acquired(const char *sem_name) {
    sem_t *sem = SEM_FAILED;
    struct timespec timeout;
    int32_t ret = 0;

    // Set semaphore timeout
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec  +=  SEM_TIMEOUT_S;
    timeout.tv_nsec +=  SEM_TIMEOUT_NS;
    sem = sem_open(sem_name, O_CREAT | O_RDWR, 0666, SEM_MAX_NUM);
    if (sem == SEM_FAILED) {
        sd_journal_print(LOG_ERR, "%s : Open Semaphore Failed, %s", __func__, strerror(errno));
        return -1;
    }
    ret = sem_timedwait(sem, &timeout);

    if (ret != 0) {
        sd_journal_print(LOG_ERR, "%s : Acquire semaphore failed, %s", __func__, strerror(errno));
        sem_close(sem);
        return -1;
    }
    sem_close(sem);
    return 0;
}

int32_t Sem_Released(const char *sem_name) {
    sem_t *sem = SEM_FAILED;
    int32_t retry_count = 0;
    int32_t ret = 0;

    sem = sem_open(sem_name, O_CREAT | O_RDWR, 0666, SEM_MAX_NUM);
    if (sem == SEM_FAILED) {
        sd_journal_print(LOG_ERR, "%s : Open Semaphore Failed, %s", __func__, strerror(errno));
        return -1;
    }

    ret = sem_post(sem);
    while ( ret != 0 && retry_count < SEM_MAX_RETRY) {
        sleep(1);
        ret = sem_post(sem);
        retry_count++;
    }

    if ( retry_count >= SEM_MAX_RETRY )
    {
        sd_journal_print(LOG_ERR, "%s : Release semaphore failed, %s", __func__, strerror(errno));
        sem_close(sem);
        return -1;
    }

    sem_close(sem);

    return 0;
}