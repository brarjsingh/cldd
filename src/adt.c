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
#include "error.h"

/* -- queue -- */

queue *
queue_new (void)
{
    queue *q = malloc (sizeof (queue));
    if (q == NULL)
        CLDD_MESSAGE("LINE: %d, malloc() failed\n", __LINE__);
    q->head = q->tail = NULL;
    return q;
}

void
queue_free (queue *q)
{
    node *e;
    while (q->head)
        q = queue_dequeue (q, (void **)&e);
    free (q);
}

queue *
queue_enqueue (queue *q, void *data)
{
    node *p = malloc (sizeof (node));

    if (p == NULL)
    {
        CLDD_MESSAGE("In %s, %s: malloc() failed\n", __FILE__, "queue_enqueue");
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

queue *
queue_dequeue (queue *q, void **data)
{
    node *p = NULL;

    if ((q == NULL) || (q->head == NULL && q->tail == NULL))
    {
        CLDD_MESSAGE("Queue is empty\n");
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

bool
queue_is_empty (queue *q)
{
    if (q == NULL || (q->head == NULL && q->tail == NULL))
        return true;
    return false;
}

int
queue_size (queue *q)
{
    int i = 0;
    node *p = NULL;

    if ((q == NULL) || (q->head == NULL && q->tail == NULL))
    {
        CLDD_MESSAGE("Queue is empty\n");
        return 0;
    }
    else if (q->head == NULL || q->tail == NULL)
    {
        CLDD_MESSAGE("There is something wrong with the queue\n"
                     "One of the head/tail is empty while other is not\n");
        return -1;
    }

    p = q->head;
    while (p)
    {
        i++;
        p = p->next;
    }
    return i;
}

/* -- llist -- */

llist *
llist_new (void)
{
    llist *l = malloc (sizeof (llist));
    if (l == NULL)
        CLDD_MESSAGE("LINE: %d, malloc() failed\n", __LINE__);
    l->link = NULL;
    return l;
}

void
llist_free (llist *l)
{
    while (l->link)
    {
        if (l->link->data != NULL)
            free (l->link->data);       /* probably shouldn't do it this way */
        free (l->link);
    }
    free (l);
}

llist *
llist_append (llist *l, void *data)
{
    node *a = NULL;
    node *b = NULL;

    a = l->link;

    if (l == NULL)
        return l;

    if (a == NULL)
    {
        a = malloc (sizeof (node));
        a->data = data;
        a->next = NULL;
        l->link = a;
    }
    else
    {
        while (a->next != NULL)
            a = a->next;

        b = malloc (sizeof (node));
        b->data = data;
        b->next = NULL;
        a->next = b;
    }

    return l;
}

llist *
llist_remove (llist *l,
              void *data,
              bool (*compare_func)(const void*, const void*))
{
    node *old = NULL;
    node *temp = NULL;

    if (l == NULL)
        CLDD_MESSAGE("Empty list, can't delete data");
    else
    {
        temp = l->link;
        while (temp != NULL)
        {
            if (compare_func (temp->data, data))
            {
                if (temp == l->link)        /* first Node case */
                    l->link = temp->next;   /* shift the header node */
                else
                    old->next = temp->next;
                free (temp);
                return l;
            }
            else
            {
                old = temp;
                temp = temp->next;
            }
        }
    }

    return l;
}

int
llist_length (llist *l)
{
    int n;
    node *p = NULL;
    for (n = 0, p = l->link; p; p = p->next, n++) ;
    return n;
}

bool
llist_is_empty (llist *l)
{
    if (l == NULL || l->link == NULL)
        return true;
    return false;
}
