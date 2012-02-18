#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//#include "../src/adt.h"

struct foo {
    int bar;
};

typedef struct _node  node;
typedef struct _llist llist;

struct _llist {
    node *link;
};

struct _node {
    node *next;
    void *data;
};

bool compare_func (const void* _a, const void* _b)
{
    // you've got to explicitly cast to the correct type
    const struct foo *a = (const struct foo *) _a;
    const struct foo *b = (const struct foo *) _b;

    if (a->bar > b->bar)
        return false;
    else
        if (a->bar == b->bar)
            return true;
        else
            return false;
}

llist *
llist_new (void)
{
    llist *l = malloc (sizeof (llist));
    if (l == NULL) ;
 //       CLDD_MESSAGE("LINE: %d, malloc() failed\n", __LINE__);
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

/* not implemented yet */
llist *
llist_remove (llist *l, void *data,
              bool(*compare_func)(const void*, const void*))
{
    if (l == NULL)
    {
        //CLDD_MESSAGE("\n\nEmpty Linked List.Cant Delete The Data.");
        printf ("Empty list, can't delete data\n");
    }
    else
    {
        node *old = NULL;
        node *temp = NULL;

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

int main (int argc, char *argv[])
{
    llist *l;
    node *il;
    struct foo *r = NULL;
    struct foo *s = malloc (sizeof (struct foo));
    struct foo *t = malloc (sizeof (struct foo));
    struct foo *u = malloc (sizeof (struct foo));

    s->bar = 10;
    t->bar = 15;
    u->bar = 20;

    l = llist_new ();

    printf ("List size: %d\n", llist_length (l));

    l = llist_append (l, (void *)s);
    l = llist_append (l, (void *)t);
    l = llist_append (l, (void *)u);

    printf ("List size: %d\n", llist_length (l));

    /* print the list */
    for (il = l->link; il; il = il->next)
    {
        r = (struct foo *)il->data;
        printf ("Found: %d\n", r->bar);
    }

    l = llist_remove (l, s, compare_func);
    printf ("Element removed\n");

    l = llist_remove (l, t, compare_func);
    printf ("Element removed\n");

    /* print the list again just for a check */
    for (il = l->link; il; il = il->next)
    {
        r = (struct foo *)il->data;
        printf ("Found: %d\n", r->bar);
    }

    l = llist_remove (l, u, compare_func);
    printf ("Element removed\n");

    printf ("List size: %d\n", llist_length (l));

    llist_free (l);

    free (s);
    free (t);
    free (u);

    return EXIT_SUCCESS;
}
