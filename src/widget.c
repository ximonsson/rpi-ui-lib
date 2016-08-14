#include "rpi_ui_widget.h"
#include <string.h>
#include <stdio.h>


typedef rpi_widget* WIDGET;


int rpi_widget_destroy (WIDGET widget)
{
	// TODO remove among parent children
	glDeleteTextures (1, &widget->texture);
	return 0;
}


void rpi_widget_translate (WIDGET widget, GLfloat x, GLfloat y, GLfloat z)
{
	widget->model[0][3] += x;
	widget->model[1][3] += y;
	widget->model[2][3] += z;
}


void rpi_widget_move (WIDGET widget, GLfloat x, GLfloat y, GLfloat z)
{
	widget->model[0][3] = x;
	widget->model[1][3] = y;
	widget->model[2][3] = z;
}


void rpi_widget_resize (WIDGET widget, GLfloat w, GLfloat h)
{
	widget->model[0][0] = w;
	widget->model[1][1] = h;
}


void rpi_widget_set_color (WIDGET widget, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	widget->r = r;
	widget->g = g;
	widget->b = b;
	widget->a = a;
}


void rpi_widget_set_parent (WIDGET widget, WIDGET new_parent)
{
	widget->parent = new_parent;
	// TODO move the widget to the correct node
}


void rpi_widget_set_source (WIDGET widget, const char* source)
{
	strncpy (widget->source, source, RPI_MAX_TEXT_LENGTH);
	// TODO load source to texture
}


void rpi_widget_set_text (WIDGET widget, const char* text, rpi_ui_text_alignment alignment)
{
	strncpy (widget->text, text, RPI_MAX_TEXT_LENGTH);
	// TODO render text to texture
	// TODO calculate coordinates depending of alignment
}
