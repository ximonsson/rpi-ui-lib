#include "rpi_ui.h"
#include "rpi_ui_widget.h"
#include <stdio.h>

int main (int argc, char** argv)
{
    bcm_host_init();

	int res;
	if ((res = rpi_init_screen ()) != 0)
	{
		fprintf (stderr, "failed to initialize screen\n");
		return 1;
	}

	rpi_widget widget;
	if ((res = rpi_widget_init (&widget, NULL)) != 0)
	{
		fprintf (stderr, "failed to create a widget\n");
		return 1;
	}
	rpi_widget_resize (&widget, .5f, .5f);
	rpi_widget_set_color (&widget, .2f, .6f, .8f, 1.f);

	int i;
    for (i = 0; i < 5; i ++)
    {
        rpi_render_screen();
        sleep(1);
    }

	rpi_widget_destroy (&widget);

    rpi_deinit_screen ();
    return 0;
}
