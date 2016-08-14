/**
 *   file: image.c
 *   description: implementation of hardware accelerated image decoding to OpenGL textures.
 *     OMX_IMAGE_PARAM_PORTFORMATTYPE: OMX_IMAGE_CodingAutoDetect
 *     http://maemo.org/api_refs/5.0/alpha/libomxil-bellagio/struct_o_m_x___i_m_a_g_e___p_a_r_a_m___p_o_r_t_f_o_r_m_a_t_t_y_p_e.html
 *
 */
#include "bcm_host.h"

/*
	SETUP
		create image_decode component
		create egl_render component
		create tunnel between them

	RENDER
		send compressed data to image_decode component
		listen to port changed
		when ports have changed, set egl_image for egl_render to use
		continue feeding buffer

*/

static void* egl_image = NULL;

int setup_image_decoder ()
{
	// TODO create components and link by tunnel
	// ilclient_create_component "image_decode"
	// ilclient_create_component "egl_render"
	return 0;
}

void port_settings_changed ()
{
	// TODO react to port settings changed
}

void buffer_filled ()
{
	// TODO fill egl_render component with decode data
}

void rpi_ui_set_image_decode_target (void* _egl_image)
{
	// TODO make sure we are not doing any decoding at the moment.
	egl_image = _egl_image;
}

int rpi_ui_decode_image (const unsigned char* data, size_t size)
{
	// TODO send data to component buffer
	return 0;
}
