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

struct options {
    int  port;
    char *log_filename;
    bool kill;
    bool daemon;
    bool verbose;
};

/**
 * parse_cmdline
 *
 * Parses the command line arguments that were passed in at launch.
 *
 * @param argc    Number of command line arguments
 * @param argv    String list of arguments
 * @param options Structure to hold the results of getopt
 */
bool
parse_cmdline (int argc, char **argv, struct options *options);

/**
 * usage
 *
 * Print the help/usage string for the daemon
 */
void
usage (char **argv);

END_C_DECLS

#endif
