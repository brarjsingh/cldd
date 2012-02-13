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

/* replace later with value taken from configuration file */
#define PID_FILE    "/var/run/cldd.pid"

/* function prototypes */
void usage (void);
void signal_handler (int sig);
void *client_manager (void *data);

int main (int argc, char *argv[])
{
    int ret;
    pthread_t master_thread;

    /* setup signal handling first */
    signal (SIGHUP,  signal_handler);
    signal (SIGINT,  signal_handler);
    signal (SIGTERM, signal_handler);
    signal (SIGQUIT, signal_handler);

    daemonize_close_stdin ();

    daemonize_init ("root", "root", PID_FILE);
    //glue_daemonize_init (&options);
    //log_init (options.verbose, options.log_stderr);

    daemonize_set_user ();

    /* true starts daemon in detached mode */
    daemonize (true);
    //setup_log_output (options.log_stderr);

    /* start the master thread for client management */
    ret = pthread_create (&master_thread, NULL, client_manager, NULL);
    if (ret != 0)
        CLDD_ERROR("Unable to create client management thread");
    pthread_join (master_thread, NULL);

    daemonize_finish ();
    //close_log_files ();

    return EXIT_SUCCESS;
}

/**
 * usage
 *
 * Print the correct usage for launching the daemon, or the help.
 */
void usage (void)
{
}

/**
 * signal_handler
 *
 * Use syslog for now, when logging facilities are available should use those
 * instead.
 *
 * @param sig The signal received
 */
void signal_handler (int sig)
{
    switch (sig) {
        case SIGHUP:
            syslog (LOG_WARNING, "Received SIGHUP signal.");
            break;
        case SIGTERM:
            syslog (LOG_WARNING, "Received SIGTERM signal.");
            break;
        default:
            syslog (LOG_WARNING, "Unhandled signal (%d) %s", strsignal(sig));
            break;
    }
}

void *client_manager (void *data)
{
    int n = 0;
    while (n<100)
    {
        syslog (LOG_WARNING, "cldd client_manager");
        sleep (1);
        n++;
    }

    pthread_exit (NULL);
}
