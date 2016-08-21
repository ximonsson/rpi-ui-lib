#include "rpi_ui.h"
#include "rpi_ui_widget.h"
#include <stdio.h>

void basic_test ()
{
	int res;
	// create a widget
	rpi_widget widget;
	rpi_widget child;
	rpi_widget child2;
	if ((res = rpi_widget_init (&widget, NULL)) != 0)
	{
		fprintf (stderr, "failed to create a widget\n");
		return exit(1);
	}
	if ((res = rpi_widget_init (&child, &widget)) != 0)
	{
		fprintf (stderr, "failed to create a widget\n");
		return exit(1);
	}
	if ((res = rpi_widget_init (&child2, &widget)) != 0)
	{
		fprintf (stderr, "failed to create a widget\n");
		return exit(1);
	}

	// setup the widget
	rpi_widget_resize (&widget, .5f, .5f);
	rpi_widget_set_color (&widget, 1.f, 0.1f, 0.2f, 1.f);

	rpi_widget_resize (&child, .25f, .25f);
	rpi_widget_move (&child, .2, .2, .0);
	rpi_widget_set_color (&child, .0, .2f, .5f, 1.0);

	rpi_widget_resize (&child2, .25f, .25f);
	rpi_widget_move (&child2, .5, .3, .0);
	rpi_widget_set_color (&child2, 1.0, .0f, .5f, 1.0);


	for (int i = 0; i < 5; i ++)
	{
		rpi_render_screen();
		sleep(1);
	}
	rpi_widget_destroy (&widget);
	rpi_widget_destroy (&child);
	rpi_widget_destroy (&child2);
}


int main (int argc, char** argv)
{
	// init
	int res;
	if ((res = rpi_init_screen ()) != 0)
	{
		fprintf (stderr, "failed to initialize screen\n");
		return 1;
	}
	basic_test ();
    rpi_deinit_screen ();
	printf ("screen deinitialized\n");
    return 0;
}
