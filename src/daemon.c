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

#include "cldd.h"
#include "daemon.h"

#include <grp.h>

/* replaced some calls to g_debug with fprintf from some sources that this was
 * pieced from, so it is possible that part related to debug printing are
 * incorrect */

/* the Unix user name which CLDD runs as */
static char *user_name;

/* the Unix user id which CLDD runs as */
static uid_t user_uid = (uid_t)-1;

/* the Unix group id which CLDD runs as */
static gid_t user_gid = (pid_t)-1;

/* the absolute path of the pidfile */
static char *pidfile;

/* whether "group" conf. option was given */
static bool had_group = false;

void
daemonize_kill (void)
{
    FILE *fp;
    int pid, ret;

    if (pidfile == NULL)
        CLDD_ERROR("no pid_file specified in the config file");

    fp = fopen (pidfile, "r");
    if (fp == NULL)
        CLDD_ERROR("unable to open pid file \"%s\": %s",
                   pidfile, strerror (errno));

    if (fscanf (fp, "%i", &pid) != 1)
        CLDD_ERROR("unable to read the pid from file \"%s\"", pidfile);

    fclose(fp);

    ret = kill (pid, SIGTERM);
    if (ret < 0)
        CLDD_ERROR("unable to kill proccess %i: %s",
                   pid, strerror (errno));

    exit (EXIT_SUCCESS);
}

void
daemonize_close_stdin (void)
{
    close (STDIN_FILENO);
    open ("/dev/null", O_RDONLY);
}

void
daemonize_set_user (void)
{
    if (user_name == NULL)
        return;

    /* technically not daemonized yet should consider replacing CLDD_ERROR
     * with a call to err_sys */

    /* set gid */
    if (user_gid != (gid_t)-1 && user_gid != getgid ())
    {
        if (setgid (user_gid) == -1) {
            CLDD_ERROR("Cannot setgid to %d: %s",
                       (int)user_gid, strerror (errno));
        }
    }

#ifdef _BSD_SOURCE
    /* init suplementary groups
     * (must be done before we change our uid)
     */
    if (!had_group && initgroups (user_name, user_gid) == -1)
    {
        fprintf (stderr, "Cannot init supplementary groups "
                         "of user \"%s\": %s\n",
                         user_name, strerror (errno));
    }
#endif

    /* set uid */
    if (user_uid != (uid_t)-1 && user_uid != getuid () &&
        setuid (user_uid) == -1)
    {
        CLDD_ERROR("Cannot change to uid of user \"%s\": %s",
                   user_name, strerror (errno));
    }
}

static void
daemonize_detach (void)
{
    /* flush all file handles before duplicating the buffers */
    fflush (NULL);

#ifdef HAVE_DAEMON

    if (daemon (0, 1))
        CLDD_ERROR("daemon() failed: %s", strerror (errno));

#elif defined(HAVE_FORK)

    /* detach from parent process */
    switch (fork ())
    {
        case -1:
            CLDD_ERROR("fork() failed: %s", strerror (errno));
        case 0:
            break;
        default:
            /* exit the parent process */
            _exit (EXIT_SUCCESS);
    }

    /* release the current working directory */
    if (chdir ("/") < 0)
        CLDD_ERROR("problems changing to root directory");

    /* detach from the current session */
    setsid ();

#else

    CLDD_ERROR("no support for daemonizing");

#endif

    fprintf (stderr, "daemonized!\n");
}

void
daemonize (bool detach)
{
    FILE *fp = NULL;

    if (pidfile != NULL)
    {
        /* do this before daemon'izing so we can fail gracefully if we can't
         * write to the pid file */
        fprintf (stderr, "opening pid file\n");
        fp = fopen (pidfile, "w+");
        if (!fp)
        {
            CLDD_ERROR("could not create pid file \"%s\": %s",
                       pidfile, strerror (errno));
        }
    }

    if (detach)
        daemonize_detach ();

    if (pidfile != NULL)
    {
        fprintf (stderr, "writing pid file\n");
        fprintf (fp, "%lu\n", (unsigned long)getpid ());
        fclose (fp);
    }
}

void
daemonize_init (const char *user, const char *group, const char *_pidfile)
{
    if (user)
    {
        struct passwd *pwd = getpwnam (user);
        if (!pwd)
            CLDD_ERROR("no such user \"%s\"", user);

        user_uid = pwd->pw_uid;
        user_gid = pwd->pw_gid;

        user_name = strdup (user);
    }

    if (group)
    {
        struct group *grp = grp = getgrnam (group);
        if (!grp)
            CLDD_ERROR("no such group \"%s\"", group);
        user_gid = grp->gr_gid;
        had_group = true;
    }

    pidfile = strdup (_pidfile);
}

void
daemonize_finish(void)
{
    if (pidfile != NULL)
        unlink (pidfile);

    free (user_name);
    free (pidfile);
}
