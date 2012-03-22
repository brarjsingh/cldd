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

#ifndef _CLDD_STREAM_H
#define _CLDD_STREAM_H

#include "common.h"

BEGIN_C_DECLS

struct stream_t {
    int port;
    int sd;
    bool open;
    gchar *guest;
    pthread_t task;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

/**
 * stream_new
 *
 * Allocate memory for new stream data
 *
 * @return The new stream data
 */
struct stream_t * stream_new (void);

/**
 * stream_free
 *
 * Free up the allocate memory of the stream data
 *
 * @param s Data to free up memory of
 */
void stream_free (struct stream_t *s);

/**
 * stream_open
 *
 * Open a port for data transmission
 *
 * @param s Stream to open a connection for
 */
void stream_open (struct stream_t *s);

/**
 * stream_close
 *
 * Close the open stream port
 *
 * @param s Stream to close the connection of
 */
void stream_close (struct stream_t *s);

/**
 * stream_thread
 *
 * Thread function to endlessly stream data to the client.
 *
 * @param data Thread data containing the client data that the stream is for
 */
void * stream_thread (void *data);

END_C_DECLS

#endif
