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

#ifndef _CLDD_H
#define _CLDD_H

#include "common.h"

BEGIN_C_DECLS

#include "cmdline.h"

/**
 * Max line length for socket reads and writes
 */
#define MAXLINE             4096

/**
 * Client backlog for socket listen
 */
#define BACKLOG             1024

/**
 * Size of epoll event queue
 */
#define EPOLL_QUEUE_LEN     256

/**
 * Default port for client management
 */
#define DEFAULT_PORT        10000

/**
 * Port number base for client transmission streams
 */
#define STREAM_PORT_BASE    10500

/**
 * File that contains the process pid, used with daemon kill
 */
#define PID_FILE    "/var/run/cldd.pid"

/* the main execution loop */
extern GMainLoop *main_loop;

/* this needs to be global for error functions */
extern struct options options;
extern bool running;

END_C_DECLS

#endif
