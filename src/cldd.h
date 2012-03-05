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

#define MAXLINE         4096        /* max text line length */
#define BACKLOG         1024
#define EPOLL_QUEUE_LEN 256

/* replace later with value taken from configuration file */
#define PID_FILE    "/var/run/cldd.pid"

/* this needs to be global for error functions */
extern struct options options;
extern bool running;

END_C_DECLS

#endif
