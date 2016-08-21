#include "rpi_ui.h"
#include "rpi_ui_widget.h"
#include <stdio.h>

#define MOVIE "movie"


static pthread_t playback_thread;
static int done = 0;


static void* play_video (void* _widget)
{
	rpi_widget_video_start ((rpi_widget*) _widget);
	done = 1;
	return NULL;
}


void video_test()
{
	int res;
	// create a widget
	rpi_widget widget;
	if ((res = rpi_widget_init (&widget, NULL)) != 0)
	{
		fprintf (stderr, "failed to create a widget\n");
		return exit(1);
	}

	// setup the widget
	pthread_mutex_t* mut;
	pthread_cond_t* cond;
	rpi_widget_resize (&widget, .5f, .5f);
	rpi_widget_set_color (&widget, 1.f, 0.f, .2f, 1.f);
	int ret = rpi_widget_video_open (&widget, MOVIE, &mut, &cond);
	if (ret != 0)
		exit(1);
	printf ("starting video\n");
	pthread_create (&playback_thread, NULL, &play_video, &widget);

	// render
	int i;
	for (i = 0; i < 1000; i ++)
	{
		pthread_mutex_lock (mut);
		pthread_cond_wait (cond, mut);
		rpi_render_screen();
		pthread_mutex_unlock (mut);
	}

	// deinit
	rpi_widget_video_stop (&widget);
	pthread_join (playback_thread, NULL);
	rpi_widget_destroy (&widget);
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
	video_test ();
    rpi_deinit_screen ();
	printf ("screen deinitialized\n");
    return 0;
}
