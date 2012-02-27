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

struct proc_stat_t {
    long long int user;
    long long int nice;
    long long int system;
    long long int idle;
    long long int iowait;
};

extern pthread_t       logging_thread;
extern pthread_cond_t  log_timer_cond;
extern pthread_mutex_t log_timer_mutex;

/* these are going to seem unnecessary at first,
 * they are intended for later development */

/* TODO: create struct log and replace server in function parameters */

void log_init (server *s, struct options *options);
void setup_log_output (server *s);
void close_log_files (server *s);
void * logging_func (void *data);

END_C_DECLS

#endif
