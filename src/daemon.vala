/**
 * Copyright (C) 2010 Control, Logging, and Data Acquisition Daemon
 * http://www.github.com/geoffjay/cldd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

using Posix;

public class Cldd.Daemon : GLib.Object {

    private pid_t pid;

    public bool running { get; set; default = false; }

    public signal void closed ();

    /**
     * Default construction.
     */
    public Daemon () {}

    /**
     * Very basic daemon initialization.
     */
    public int init () {
        /* Fork off parent process */
        pid = fork ();

        if (pid < 0)
            exit (EXIT_FAILURE);

        /* Allow the parent to terminate */
        if (pid > 0)
            exit (EXIT_SUCCESS);

        /* Make the child process the leader */
        if (setsid () < 0)
            exit (EXIT_FAILURE);

        /* Setup signals */
        Posix.signal (SIGCHLD, SIG_IGN);
        Posix.signal (SIGHUP, shutdown);
        Posix.signal (SIGINT, shutdown);

        /* Fork off again */
        pid = fork ();

        if (pid < 0)
            exit (EXIT_FAILURE);

        /* Allow the parent to terminate */
        if (pid > 0)
            exit (EXIT_SUCCESS);

        /* Set the new file permissions */
        umask (0);

        /* Move to root */
        chdir ("/");

        /* Close any file descriptors that are open */
        for (var x = sysconf (_SC_OPEN_MAX); x > 0; x--) {
            close ((int)x);
        }

        /* Open a log file to write to */
        openlog ("cldd", LOG_PID, LOG_DAEMON);
        syslog (LOG_NOTICE, "(CLDD) daemon initialized");

        running = true;

        return 0;
    }

    public int interrupt () {
        raise (SIGINT);
        return 0;
    }

    public void shutdown () {
        syslog (LOG_NOTICE, "(CLDD) shutting down");
        /* Cleanup */
        pid_file_remove ();
        closelog ();
        running = false;
        /* Raise the signal to allow main process to close gracefully */
        closed ();
    }

    public void pid_file_remove () {
    }
}
