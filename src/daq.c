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

#include "common.h"

#include "daq.h"

struct daq_t *
daq_new (void)
{
    struct daq_t *daq = malloc (sizeof (struct daq_t));

    daq->is_initialized = false;

    return daq;
}

struct daq_t *
daq_new_from_builder (CldBuilder *builder)
{
    struct daq_t *daq = malloc (sizeof (struct daq_t));

    /* default call will return the first available */
    daq->c_daq = cld_builder_get_default_daq (builder);
    daq->c_control = cld_builder_get_default_control (builder);
    /* read all of the object lists as TreeMaps */
    daq->c_calibrations = cld_builder_get_calibrations (builder);
    daq->c_channels = cld_builder_get_channels (builder);

    daq->is_initialized = true;

    return daq;
}

void
daq_free (struct daq_t *daq)
{
    /* don't know if this frees the memory in maps or not but gee doesn't
     * include free functions for its collections */
    gee_map_clear (daq->c_calibrations);
    gee_map_clear (daq->c_channels);

    daq->is_initialized = false;

    free (daq);
}
