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
 * A structure that contains data needed for logging
 */
struct log_t {
    /*@{*/
    bool active;            /**< whether or the log task is running */
    bool reload;
    CldLog *c_log;          /**< data for logging */
    GThread *task;
    /*@}*/
};

/**
 * Process statistics to log
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

/**
 * Statistics log
 */
struct stat_log_t {
    bool active;
    bool reload;
    struct proc_stat_t *stat;
    GThread *task;
};

extern GThread *log_task;

/* -- start of new log section -- */

struct log_t * log_new (void);
void log_free (struct log_t *log);
int log_start (struct log_t *log);
int log_pause (struct log_t *log);
int log_stop (struct log_t *log);

/**
 * Thread function that reads current performance stats and logs them to a file
 * using a timer loop with frequency set by configuration file.
 *
 * @param data Server data with info of server and file pointers to write to
 */
void * log_thread (gpointer data);

/* TODO: create struct log and replace server in function parameters */

/**
 * Initialize the server performance logging. This was used for an early test
 * and isn't needed anymore.
 *
 * @param s       Server data with info of server and file pointers to write to
 * @param options Command line options given at launch
 */
void stat_log_new (server *s, struct options *options);

/**
 * Free up the memory that was used for the server stats log.
 *
 * @param log The memory to free
 */
void stat_log_free (struct stat_log_t *log);

/**
 * Wrapper function to start the log thread.
 *
 * @param s Server data with pid of server and file pointers to write to
 */
void stat_log_setup_output (server *s);

/**
 * Stops the logging thread and closes any files that were opened for the log
 * process.
 *
 * @param s Server data with info of server and file pointers to write to
 */
void stat_log_file_close (server *s);

/**
 * Thread function that reads current performance stats and logs them to a file
 * using a timer loop with frequency set by configuration file.
 *
 * @param data Server data with info of server and file pointers to write to
 */
void * stat_log_thread (gpointer data);

END_C_DECLS

#endif
