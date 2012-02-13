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

#include <common.h>

#include "main.h"
#include "client.h"
#include "conf.h"
#include "daemon.h"
#include "utils.h"
#include "cldd_error.h"

int main (int argc, char *argv[])
{
    daemonize_close_stdin ();

    daemonize_init ("root", "root", "/var/run/cldd.pid");
    //glue_daemonize_init (&options);
    //log_init (options.verbose, options.log_stderr);

    daemonize_set_user ();

    /* start attached initially */
    daemonize (false);
    //setup_log_output (options.log_stderr);

    /* start the master thread for client management */

    daemonize_finish ();
    //close_log_files ();

    return EXIT_SUCCESS;
}
