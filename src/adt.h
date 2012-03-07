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

/**
 * @file adt.h
 * Function declarations for abstract data types used by CLDD.
 */

#ifndef _CLDD_ADT_H
#define _CLDD_ADT_H

#include "common.h"

BEGIN_C_DECLS

typedef struct _node  node;
typedef struct _llist llist;
typedef struct _queue queue;

struct _llist {
    node *link;
};

struct _queue {
    node *head;     /* front */
    node *tail;     /* rear  */
};

struct _node {
    node *next;
    void *data;
};

/**
 * queue_new
 */
queue *
queue_new (void);

/**
 * queue_free
 */
void
queue_free (queue *q);

/**
 * queue_enqueue
 */
queue *
queue_enqueue (queue *q, void  *data);

/**
 * queue_dequeue
 */
queue *
queue_dequeue (queue *q, void **data);

/**
 * queue_is_empty
 */
bool
queue_is_empty (queue *q);

/**
 * queue_size
 */
int
queue_size (queue *q);

/**
 *
 */
llist *
llist_new (void);

/**
 *
 */
void
llist_free (llist *l, void (*free_func)(void *));

/**
 *
 */
llist *
llist_append (llist *l, void *data);

/**
 *
 */
llist *
llist_remove (llist *l,
              void *data,
              bool (*compare_func)(const void*, const void*));

/**
 *
 */
int
llist_length (llist *l);

/**
 *
 */
bool
llist_is_empty (llist *l);

END_C_DECLS

#endif
