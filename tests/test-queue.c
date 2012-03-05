#include "common.h"
#include "adt.h"

struct foo {
    int bar;
};

int main (int argc, char *argv[])
{
    queue *q;
    node *iq;
    struct foo *r = NULL;
    struct foo *s = malloc (sizeof (struct foo));
    struct foo *t = malloc (sizeof (struct foo));
    struct foo *u = malloc (sizeof (struct foo));

    s->bar = 10;
    t->bar = 15;
    u->bar = 20;

    q = queue_new ();
    q = queue_enqueue (q, (void *)s);
    q = queue_enqueue (q, (void *)t);
    q = queue_enqueue (q, (void *)u);

    for (iq = q->head; iq; iq = iq->next)
    {
        r = (struct foo *)iq->data;
        printf ("Found: %d\n", r->bar);
        //r = NULL;
    }

    while (q->head)
    {
        q = queue_dequeue (q, (void **)&r);
        printf ("Found: %d\n", r->bar);
        //r = NULL;
    }

    queue_free (q);

    free (s);
    free (t);
    free (u);

    return EXIT_SUCCESS;
}
