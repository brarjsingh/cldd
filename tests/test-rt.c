/**
 * Compile with:
 * $ gcc -o test-rt test-rt.c -lrt -lpthread
 *
 * Run with:
 * $ sudo ./test-rt
 *
 * High resolution timer allows up to 1ms timing, therefore the max timer
 * rate is 1kHz.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sched.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>

static bool
rt_memlock (void)
{
    int ret;
    struct sched_param sp;

    if (geteuid () == 0)
    {
        memset (&sp, 0, sizeof (sp));
        sp.sched_priority = sched_get_priority_max (SCHED_FIFO);
        if ((ret = sched_setscheduler (0, SCHED_FIFO, &sp)) == -1)
            return false;
        if ((ret = mlockall (MCL_CURRENT | MCL_FUTURE)) == -1)
            return false;
        /* memory locking is complete if we made it here */
        return true;
    }
    return false;
}

static bool
rt_hrtimers (void)
{
    struct timespec ts;

    clock_getres (CLOCK_MONOTONIC, &ts);
    printf ("Resolution: %ld secs, %ld nsecs\n", ts.tv_sec, ts.tv_nsec);

    return true;
}

static inline void
ts_norm (struct timespec *ts)
{
    while (ts->tv_nsec >= 1000000000)
    {
        ts->tv_nsec -= 1000000000;
        ts->tv_sec++;
    }
}

static inline int64_t
calcdiff (struct timespec t1, struct timespec t2)
{
    int64_t diff;
    diff = 1000000 * (long long)((int) t1.tv_sec - (int) t2.tv_sec);
    diff += ((int) t1.tv_nsec - (int) t2.tv_nsec) / 1000;
    return diff;
}

static inline int64_t
calcdiff_ns (struct timespec t1, struct timespec t2)
{
    int64_t diff;
    diff = 1000000000 * (long long)((int) t1.tv_sec - (int) t2.tv_sec);
    diff += ((int) t1.tv_nsec - (int) t2.tv_nsec);
    return diff;
}

void *
rt_thread (void *data)
{
    int policy, ret = 0, cnt = 0;
    struct sched_param sp;
    struct timespec now, interval, next;

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

    interval.tv_sec = 0;
    interval.tv_nsec = 1000000;     /* 1kHz */

    clock_gettime (CLOCK_REALTIME, &now);

    next = now;
    next.tv_sec += interval.tv_sec;
    next.tv_nsec += interval.tv_nsec;
    ts_norm (&next);

    pthread_getschedparam (pthread_self(), &policy, &sp);

    printf ("Thread: running at %s/%d\n",
            (policy == SCHED_FIFO ? "FIFO"
                : (policy == SCHED_RR ? "RR"
                : (policy == SCHED_OTHER ? "OTHER"
                : "unknown"))), sp.sched_priority);

    while (true)
    {
        clock_gettime (CLOCK_REALTIME, &now);
        printf ("%12ld.%12ld ...\n", now.tv_sec, now.tv_nsec%1000000000);

        pthread_mutex_lock (&mutex);
        while (true)
        {
            ret = pthread_cond_timedwait (&cond, &mutex, &next);
            if (ret == ETIMEDOUT)
                break;
        }
        pthread_mutex_unlock (&mutex);

        next.tv_sec += interval.tv_sec;
        next.tv_nsec += interval.tv_nsec;
        ts_norm (&next);

        if (++cnt == 1000) break;
    }

    return NULL;
}

int
main (int argc, char *argv[])
{
    pthread_t thread;
    pthread_attr_t attr;
    struct sched_param sp;

    /* lock process into memory to avoid paging */
    if (rt_memlock () == false)
        fprintf (stderr, "rt_memlock failed, real time not enabled\n");

    /* check whether or not high resolution timers are enabled */
    if (rt_hrtimers () == false)
        fprintf (stderr, "rt_hrtimers failed, real time not enabled\n");

    pthread_attr_init (&attr);
    pthread_attr_setinheritsched (&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy (&attr, SCHED_FIFO);
    memset (&sp, 0, sizeof (sp));
    sp.sched_priority = 50;
    pthread_attr_setschedparam (&attr, &sp);

    printf ("Starting thread\n");
    pthread_create (&thread, NULL, rt_thread, NULL);
    pthread_join (thread, NULL);

    return false;
}
