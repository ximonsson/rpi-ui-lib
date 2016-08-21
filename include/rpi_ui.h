/**
 * File: ui.h
 * Description: Library for creating a user interface for an application running on Raspberry Pi.
 */
#ifndef RPI_UI_H_
#define RPI_UI_H_

#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"


/**
 *  Init screen for OpenGL rendering.
 *  Returns non-zero on failure.
 */
int rpi_init_screen () ;

/**
 *  Deinitialize components for rendering.
 *  Returns non-zero on failure.
 */
int rpi_deinit_screen () ;

/**
 *  Render screen.
 *  Goes through all visible widgets and renders them to screen.
 */
void rpi_render_screen () ;

/**
 *  Create a new KHR Image from a OpenGL texture,
 *  that can be used as render buffer for
 *  image/video decoding through OpenMAX library.
 */
int rpi_create_image (GLuint /* texture */, void** /* egl_image */) ;

#endif
