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

#ifndef _CLDD_ERROR_H
#define _CLDD_ERROR_H

#include "common.h"

BEGIN_C_DECLS

/* These are a mix of some useful error facilities from mpd and Steven's unp. */

/**
 * err_doit
 *
 * Print message and return to caller.
 *
 * @param errnoflag Caller specified flags for error number
 * @param level     Caller specified logging lebel to use
 * @param fmt       Formatted print string
 * @param ap        VA args list
 */
static void err_doit (int errnoflag, int level, const char *fmt, va_list ap);

/**
 * err_ret
 *
 * Nonfatal error related to system call
 * Print message and return
 *
 * @param fmt Formatted print string
 * @param ... VA args list
 */
void err_ret (const char *fmt, ...);

/**
 * err_sys
 *
 * Fatal error related to system call
 * Print message and terminate
 *
 * @param fmt Formatted print string
 * @param ... VA args list
 */
void err_sys (const char *fmt, ...);

/**
 * err_dump
 *
 * Fatal error related to system call
 * Print message, dump core, and terminate
 *
 * @param fmt Formatted print string
 * @param ... VA args list
 */
void err_dump (const char *fmt, ...);

/**
 * err_msg
 *
 * Nonfatal error unrelated to system call
 * Print message and return
 *
 * @param fmt Formatted print string
 * @param ... VA args list
 */
void err_msg (const char *fmt, ...);

/**
 * err_quit
 *
 * Fatal error unrelated to system call
 * Print message and terminate
 *
 * @param fmt Formatted print string
 * @param ... VA args list
 */
void err_quit (const char *fmt, ...);

/* These macros are used as an intermediate step to proper error handling
 * in cldd. */

#define CLDD_ERROR(...) \
    do { \
        err_quit (__VA_ARGS__); \
    } while (0)

#define CLDD_MESSAGE(...) \
    do { \
        err_msg (__VA_ARGS__); \
    } while (0)

END_C_DECLS

#endif
