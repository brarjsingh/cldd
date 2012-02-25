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

#include "cmdline.h"
#include "conf.h"
#include "error.h"

/* for pulling options from a config file if I ever get around to it */
#define USER_CONFIG_FILE_LOCATION1    ".clddconf"
#define USER_CONFIG_FILE_LOCATION2    ".cldd/cldd.conf"

static const char *summary =
    "Control Logging and Data Acquisition Daemon\n"
    "a daemon for data acquistion and control.";

static void
version (void)
{
    printf (PACKAGE " (CLDD: Control Logging and Data Acquisition Daemon) "
            VERSION " \n\n"
            "Copyright (C) 2012 Geoff Johnson <geoff.jay@gmail.com>\n"
            "This is free software; see the source for copying conditions.\n"
            "There is NO warranty; not even MERCHANTABILITY or FITNESS FOR A"
            "PARTICULAR PURPOSE.\n");
    exit (EXIT_SUCCESS);
}

void
usage (char **argv)
{
    printf ("Usage:\n\t%s [OPTION...]\n\n"
            "%s\n\n"
            "Help Options:\n"
            "\t-h|?, --help\    show help options\n\n"
            "Application Options:\n"
            "\t-p, --port       the port to use for the daemon\n"
            "\t-k, --kill       kill the currently running cldd session\n"
            "\t-d, --daemon     detach from the console\n"
            "\t-v, --verbose    verbose logging\n"
            "\t-V, --Version    print version number\n",
            argv[0], summary);
    exit (EXIT_SUCCESS);
}

bool
parse_cmdline (int argc, char **argv, struct options *options)
{
    int opt = 0;
    int opt_index = 0;

    static const char * opt_string = ":vdp:l:Vkh?";
    static const struct option cmd_options[] = {
        /* options that set a flag */
        { "verbose", no_argument,       NULL, 'v' },
        { "daemon",  no_argument,       NULL, 'd' },
        { "port",    required_argument, NULL, 'p' },
        { "logfile", required_argument, NULL, 'l' },
        { "version", no_argument,       NULL, 'V' },
        { "kill",    no_argument,       NULL, 'k' },
        { "help",    no_argument,       NULL, 'h' },
        { NULL,      no_argument,       NULL, 0   }
    };

    /* set defaults */
    options->port = 10000;
    options->log_filename = malloc (sizeof (32));
    strcpy (options->log_filename, "log.dat");

    /* should include error checking later */

    opt = getopt_long (argc, argv, opt_string, cmd_options, &opt_index);
    while (opt != -1)
    {
        switch (opt)
        {
            case 'v':
                options->verbose = true;
                break;
            case 'd':
                options->daemon = true;
                break;
            case 'p':
                options->port = atoi (optarg);
                break;
            case 'l':
                free (options->log_filename);
                options->log_filename = malloc (sizeof (optarg));
                strcpy (options->log_filename, optarg);
                break;
            case 'V':
                version ();
                break;
            case 'k':
                options->kill = true;
                break;
            case ':':   /* missing option argument */
               fprintf (stderr, "%s: option `-%c' requires an argument\n",
                        argv[0], optopt);
            case 'h':
            case '?':
                usage (argv);
                break;
            default:
                /* should never reach this */
                break;
        }

        opt = getopt_long (argc, argv, opt_string, cmd_options, &opt_index);
    }

    return true;
}
