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

#ifndef _CLDD_LOG_H
#define _CLDD_LOG_H

#include "common.h"

BEGIN_C_DECLS

#include "cmdline.h"
#include "server.h"

/**
 * Process statistics to log.
 */
struct proc_stat_t {
    /*@{*/
    long long int user;     /**< cpu user */
    long long int nice;     /**< cpu nice */
    long long int system;   /**< cpu system */
    long long int idle;     /**< cpu idle */
    long long int iowait;   /**< cpu iowait */
    /*@}*/
};

extern GThread *log_task;

/* TODO: create struct log and replace server in function parameters */

/**
 * Initialize the server performance logging. This was used for an early test
 * and isn't needed anymore.
 *
 * @param s       Server data with info of server and file pointers to write to
 * @param options Command line options given at launch
 */
void log_init (server *s, struct options *options);

/**
 * Wrapper function to start the log thread.
 *
 * @param s Server data with pid of server and file pointers to write to
 */
void setup_log_output (server *s);

/**
 * Stops the logging thread and closes any files that were opened for the log
 * process.
 *
 * @param s Server data with info of server and file pointers to write to
 */
void close_log_files (server *s);

/**
 * Thread function that reads current performance stats and logs them to a file
 * using a 10Hz timer loop.
 *
 * @param data Server data with info of server and file pointers to write to
 */
void * log_thread (void *data);

END_C_DECLS

#endif
