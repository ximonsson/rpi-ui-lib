#include "rpi_ui_utils.h"
#include <stdio.h>


void test_struct ()
{
	printf ("Testing queueing structs\n");
	typedef struct node
	{
		int x;
		int y;
	} node;

	rpi_ui_queue q;
	rpi_ui_queue_init (&q, sizeof (node*));

	node a = {1,1};
	node b = {2,3};
	node c = {4,1};

	rpi_ui_queue_push (&q, &a);
	rpi_ui_queue_push (&q, &b);
	rpi_ui_queue_push (&q, &c);

	node* foo;
	rpi_ui_queue_pop (&q, &foo);
	printf (" Node (%d, %d)\n", foo->x, foo->y);
	rpi_ui_queue_pop (&q, &foo);
	printf (" Node (%d, %d)\n", foo->x, foo->y);
	rpi_ui_queue_pop (&q, &foo);
	printf (" Node (%d, %d)\n", foo->x, foo->y);

	rpi_ui_queue_destroy (&q);
}

void test_int ()
{
	printf ("Testing queueing integers\n");
	rpi_ui_queue q;
	rpi_ui_queue_init (&q, sizeof (int));

	for (int i = 0; i < 5; i ++)
	{
		int n = i + 10;
		rpi_ui_queue_push (&q, (void*) n);
	}

	for (int i = 0; i < 5; i ++)
	{
		int n;
		rpi_ui_queue_pop (&q, &n);
		printf (" %d", n);
	}
	printf ("\n");

	rpi_ui_queue_destroy (&q);
}

int main (int argc, char** argv)
{
	test_struct();
	test_int();



	return 0;
}
