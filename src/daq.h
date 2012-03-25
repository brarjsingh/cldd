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

#ifndef _CLDD_DAQ_H
#define _CLDD_DAQ_H

#include "common.h"

BEGIN_C_DECLS

/**
 * A structure that contains data needed for data acquisition
 */
struct daq_t {
    /*@{*/
    GList *ai_chans;    /**< analog input channel list */
    GList *ao_chans;    /**< analog output channel list */
    GList *di_chans;    /**< digital input channel list */
    GList *do_chans;    /**< digital output channel list */
    GList *cnt_chans;   /**< counter channel list */
    GList *uart_devs;   /**< uart device list */
    /*@}*/
};

/**
 * Thread function for analog input measurements from hardware devices.
 *
 * @param data ... Not sure yet.
 */
void * ai_thread (gpointer data);

/**
 * Not really sure how I intend to use ao, di, and do threads. Possibly have
 * and command queue that the threads monitor with a thread condition and
 * execute when not empty.
 */

void * ao_thread (gpointer data);
void * di_thread (gpointer data);
void * do_thread (gpointer data);

/**
 * The cnt and uart threads will likely be setup to monitor changes in those
 * devices or respond to user requests that are dumped into a command queue.
 */

void * cnt_thread (gpointer data);
void * uart_thread (gpointer data);

END_C_DECLS

#endif
