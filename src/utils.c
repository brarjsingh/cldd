/**
 * Copyright (C) 2010 Geoff Johnson <geoff.jay@gmail.com>
 *
 * This file is part of cldd.
 *
 * cldd is free software; you can redistribute it and/or modify
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "common.h"

#include "cldd.h"
#include "utils.h"

/*
int set_nonblocking (int fd)
{
    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
        flags = 0;
    return fcntl (fd, F_SETFL, flags | O_NONBLOCK);
}
*/

bool
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

bool
rt_hrtimers (void)
{
    struct timespec ts;

    clock_getres (CLOCK_MONOTONIC, &ts);
    if (ts.tv_nsec > 0)
        return true;

    return false;
}

