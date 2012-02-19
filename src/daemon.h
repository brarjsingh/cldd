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

#ifndef _CLDD_DAEMON_H
#define _CLDD_DAEMON_H

#include "common.h"

BEGIN_C_DECLS

#include "error.h"

/**
 * daemonize_init
 *
 * Setup user, group, and process lock file for daemon.
 *
 * @param user    The user to launch the daemon as
 * @param group   The group to use for the daemon
 * @param pidfile The pid lock file for the daemon process
 */
void
daemonize_init (const char *user, const char *group, const char *pidfile);

/**
 * daemonize_finish
 *
 * Close the daemon and kill the process.
 */
void
daemonize_finish (void);

/**
 * daemonize_kill
 *
 * Kill the CLDD which is currently running, pid determined from the
 * pid file.
 */
void
daemonize_kill (void);

/**
 * daemonize_close_stdin
 *
 * Close stdin (fd 0) and re-open it as /dev/null.
 */
void
daemonize_close_stdin (void);

/**
 * daemonize_set_user
 *
 * Change to the configured Unix user.
 */
void
daemonize_set_user (void);

/**
 * daemonize
 *
 * Daemonize the server in either attached or detached mode.
 *
 * @param detach Whether or not to detach the server process
 */
void
daemonize (bool detach);

END_C_DECLS

#endif
