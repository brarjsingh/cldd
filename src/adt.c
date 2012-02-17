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

#include "adt.h"

queue * queue_new (void)
{
    queue *q = malloc (sizeof (queue));
    if (q == NULL) ;
        CLDD_MESSAGE("LINE: %d, malloc() failed\n", __LINE__);
    q->head = q->tail = NULL;
    return q;
}

void queue_free (queue *q)
{
    node *e;
    while (q->head)
        q = queue_dequeue (q, e);
    free (q);
}

queue * queue_enqueue (queue *q, void *data)
{
    node *p = malloc (sizeof (node));

    if (p == NULL)
    {
        CLDD_MESSAGE("In %s, %s: malloc() failed\n", __FILE__, "queue_enq");
        return NULL;
    }

    p->data = data;
    p->next = NULL;

    if (q == NULL)
    {
        CLDD_MESSAGE("Queue not initialized\n");
        free (p);
        return q;
    }
    else if (q->head == NULL && q->tail == NULL)
    {
        /* empty list */
        q->head = q->tail = p;
        return q;
    }
    else if (q->head == NULL || q->tail == NULL)
    {
        CLDD_MESSAGE("There is something wrong the assignment of head/tail "
                     "to the queue\n");
        free (p);
        return NULL;
    }
    else
    {
        /* not empty, adding element to tail */
        q->tail->next = p;
        q->tail = p;
    }

    return q;
}

queue * queue_dequeue (queue *q, void **data)
{
    node *p = NULL;

    if ((q == NULL) || (q->head == NULL && q->tail == NULL))
    {
        CLDD_MESSAGE("Queue is empty\n");
        printf ("queue is empty\n");
        return q;
    }
    else if (q->head == NULL || q->tail == NULL)
    {
        CLDD_MESSAGE("There is something wrong with the queue\n"
                     "One of the head/tail is empty while other is not\n");
        return q;
    }

    *data = q->head->data;
    p = q->head;
    q->head = q->head->next;
    if (q->tail == p)
        q->tail = q->tail->next;
    free (p);
    p = NULL;

    /* The element tail was pointing to was freed, so we need an update */
    if (q->head == NULL)
        q->tail = q->head;

    return q;
}
