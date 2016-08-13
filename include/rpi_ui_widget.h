#ifndef RPI_UI_WIDGET_H_
#define RPI_UI_WIDGET_H_

#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "bcm_host.h"


#define RPI_MAX_TEXT_LENGTH     1024


typedef enum alignment
{
	text_center,
	text_left,
	text_right,
}
rpi_ui_text_alignment;


/**
 *  RPI UI Widget.
 *  Can draw rectangle, write text, display image or even play video,
 *  but only one of them at a time as it only draws one texture.
 *
 *  All dimension and position values are between 0.0 and 1.0, and are
 *  calculated relatively to it's parent. If the widget has no parent
 *  it defaults to the root window.
 *
 *  Image and video network sources are supported.
 *
 *  Must be initialized with rpi_widget_init and deallocated with
 *  rpi_widget_destroy when done.
 */
typedef struct rpi_widget
{
	/* position */
	GLfloat x;
	GLfloat y;
	GLfloat z;

	/* color */
	GLfloat r;
	GLfloat g;
	GLfloat b;
	GLfloat a;

	/* dimensions */
	GLfloat width;
	GLfloat height;

	/* model matrix */
	GLfloat model[4][4];

	GLuint  texture;

	char    text[RPI_MAX_TEXT_LENGTH];

	char    source[RPI_MAX_TEXT_LENGTH];

	int     visible;

	struct rpi_widget* parent;
}
rpi_widget ;

/**
 *  Allocates and sets up a new widget.
 *  Remember rpi_widget_destroy when done.
 *  Returns non-zero on failure.
 */
int rpi_widget_init (rpi_widget* /* widget */, rpi_widget* /* parent */) ;

/**
 *  Deallocates a widget.
 *  Returns non-zero on failure.
 */
int rpi_widget_destroy (rpi_widget* /* widget */) ;

/**
 *  Draw a widget to screen.
 *  Only if visible though.
 */
void rpi_widget_draw (rpi_widget* /* widget */) ;

/**
 *  Resize the widget to new dimensions.
 */
void rpi_widget_resize (rpi_widget* /* widget */,
                        GLfloat     /* new_width */,
                        GLfloat     /* new_height */) ;

/**
 *  Translate the widget relative to its current position.
 */
void rpi_widget_translate (rpi_widget*  /* widget */,
                           GLfloat      /* x */,
                           GLfloat      /* y */,
                           GLfloat      /* z */) ;

/**
 *  Move a widget to a new absolute position.
 */
void rpi_widget_move (rpi_widget* /* widget */,
                      GLfloat     /* x */,
                      GLfloat     /* y */,
                      GLfloat     /* z */) ;

/**
 *  Rotate a widget for an angle specified in radians.
 *  At the moment only 2D rotating, through Y-axis, is supported.
 */
void rpi_widget_rotate (rpi_widget* /* widget */,
                        GLfloat     /* angle */,
                        GLfloat     /* axis */) ;

/**
 *  Change media source of a widget.
 *  Forces the widget to refresh and update its content.
 *  Setting media source is the same as changing its texture.
 */
void rpi_widget_set_source (rpi_widget* /* widget */, const char* /* source */) ;

/**
 *  Sets text contents of the widget.
 *  If any media is displayed on the widget it will be replaced with the
 *  supplied text instead.
 */
void rpi_widget_set_text (rpi_widget* /* widget */, const char* /* text */, rpi_ui_text_alignment /* alignment */) ;

/**
 *  Change color and alpha value of a widget.
 */
void rpi_widget_set_color (rpi_widget* /* widget */,
                           GLfloat /* r */,
                           GLfloat /* g */,
                           GLfloat /* b */,
                           GLfloat /* a */);

/**
 *  Change widget parent.
 */
void rpi_widget_set_parent (rpi_widget* /* widget */, rpi_widget* /* new_parent */) ;

#endif // RPI_UI_WIDGET_H_
