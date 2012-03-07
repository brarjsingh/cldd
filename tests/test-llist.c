#include "common.h"
#include "adt.h"

struct foo {
    int bar;
};

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
