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

#ifndef _CLDD_CMDLINE_H
#define _CLDD_CMDLINE_H

#include "common.h"

BEGIN_C_DECLS

/**
 * Command line options read from the user.
 */
struct options {
    /*@{*/
    int  port;          /**< port number to use with the server */
    char *log_filename; /**< filename to write server statistics to */
    char *cldd_config;  /**< file to load daemon configuration from */
    char *cld_config;   /**< file to load CLD configuration from */
    bool kill;          /**< kill flag used to send shutdown signal to daemon */
    bool daemon;        /**< flag to tell process whether or not to daemonize */
    bool verbose;       /**< verbose command line output for debugging */
    /*@}*/
};

/**
 * Parses the command line arguments that were passed in at launch.
 *
 * @param argc    Number of command line arguments
 * @param argv    String list of arguments given by user
 * @param options Structure to hold the results of getopt
 */
bool parse_cmdline (int argc, char **argv, struct options *options);

/**
 * Print the help/usage string for the daemon
 *
 * @param argv String list of arguments given by user
 */
void usage (char **argv);

END_C_DECLS

#endif
