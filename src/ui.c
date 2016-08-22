#include "rpi_ui.h"
#include "rpi_ui_widget.h"
#include "rpi_ui_utils.h"
#include "rpi_mp.h"
#include "bcm_host.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define LCD 0
#define FRAGMENT_SHADER_FILE "src/shaders/fragment.glsl"
#define VERTEX_SHADER_FILE "src/shaders/vertex.glsl"
#define N_CHILDREN_ALLOC 8

typedef rpi_widget* WIDGET;

/**
 *  Keep a tree of the widgets in the window.
 *  This struct represents a node in the tree.
 */
typedef struct widget_node
{
	WIDGET widget;
	int    nchildren;
	int    capacity;
	struct widget_node** children;
}*
NODE;

/* root node of the widgets */
static struct widget_node root;

/* Screen dimensions */
static uint32_t     width,
                    height;

/* EGL components */
static EGLDisplay   display;
static EGLSurface   surface;
static EGLContext   context;

/* OpenGL components */
static GLuint       program,
                    vertexshader,
                    fragmentshader,
                    color_uniform,
                    vertex_vbo,
                    texture_vbo,
                    model_uniform,
                    view_uniform,
                    default_texture;

/* Default texture coordinates */
static GLfloat texture_coords[4 * 2] =
{
	0.f,  1.f,
	1.f,  1.f,
	0.f,  0.f,
	1.f,  0.f
};

/* Default vertex coordinates. */
static GLfloat vertex_coords[4 * 3] =
{
	0.0,  0.0,  0.0,
	1.0,  0.0,  0.0,
	0.0,  1.0,  0.0,
	1.0,  1.0,  0.0
};

/* Basic eye matrix used as default */
static GLfloat eye[4][4] =
{
	{1.f, 0.f, 0.f, 0.f},
	{0.f, 1.f, 0.f, 0.f},
	{0.f, 0.f, 1.f, 0.f},
	{0.f, 0.f, 0.f, 1.f},
};

/**
 *  Render widget connected to node and continue down its children.
 *  Depth first style.
*/
static void render_sub_tree (NODE node, GLfloat view[4][4])
{
	// TODO this is a recursive function, maybe it shouldn't to save on performance.

	glUniformMatrix4fv (view_uniform, 1, GL_FALSE, (GLfloat*) view);
	rpi_widget_draw (node->widget);
	// render children within the view of this widget.
	GLfloat modelview[4][4];
	for (int i = 0; i < node->nchildren; i ++)
	{
		if (!node->children[i]->widget->visible) // make sure widget is visible
			continue;

		// calculate new view matrix for children, and load to GPU.
		rpi_ui_matrix_mul (view, node->widget->model, modelview);
		// render child
		render_sub_tree (node->children[i], modelview);
	}
}

/**
 *  Search the widget tree for the node corresponding to the widget.
 */
static NODE find_node (WIDGET widget)
{
	// TODO should there be one queue that is reused each time this function is called?
	rpi_ui_queue q;
	rpi_ui_queue_init (&q, sizeof (NODE));
	for (int i = 0; i < root.nchildren; i ++)
		rpi_ui_queue_push (&q, root.children[i]);

	NODE node = NULL, tmp = NULL;
	int found = 0;
	while (!found && q.length)
	{
		rpi_ui_queue_pop (&q, &tmp);
		if (tmp->widget == widget) // we found the node
		{
			node = tmp;
			found = 1;
		}
		else // push children to queue
		{
			for (int i = 0; i < tmp->nchildren; i ++)
				rpi_ui_queue_push (&q, tmp->children[i]);
		}
	}
	rpi_ui_queue_destroy (&q);
	return node;
}

/**
 *  Append the node to the parents children.
 */
static void append_to_children (NODE node, NODE child)
{
	if (node->nchildren == node->capacity)
	{
		// we need to allocate a new larger buffer for children.
		NODE* children = node->children;
		node->capacity += N_CHILDREN_ALLOC;
		node->children = malloc (sizeof (NODE) * node->capacity);
		memcpy (node->children, children, node->nchildren * sizeof (NODE));
		free (children);
	}
	// add node to children.
	node->children[node->nchildren] = child;
	node->nchildren ++;
}

/**
 *  Create a new node in the widget tree, specifying its parent.
 *  Returns a reference to the new node.
 */
static NODE new_node (WIDGET widget, NODE parent)
{
	// allocate a new node
	NODE new_node = malloc (sizeof (struct widget_node));
	new_node->nchildren   = 0;
	new_node->capacity    = N_CHILDREN_ALLOC;
	new_node->widget      = widget;
	new_node->children    = malloc (sizeof (NODE) * N_CHILDREN_ALLOC);
	// append to parent
	append_to_children (parent, new_node);
	return new_node;
}

/**
 *	Initialize EGL.
 */
static int init_egl ()
{
	printf ("initializing EGL\n");
	bcm_host_init ();

	static
	EGL_DISPMANX_WINDOW_T       native_window;
	DISPMANX_ELEMENT_HANDLE_T   dispman_element;
	DISPMANX_DISPLAY_HANDLE_T   dispman_display;
	DISPMANX_UPDATE_HANDLE_T    dispman_update;

	EGLBoolean  result;
	EGLConfig   config;
	EGLint      num;
	int         success;

	static const EGLint attribute_list[] =
	{
		EGL_RED_SIZE,    8,
		EGL_GREEN_SIZE,  8,
		EGL_BLUE_SIZE,   8,
		EGL_ALPHA_SIZE,  8,
		EGL_DEPTH_SIZE, 16,
		EGL_SAMPLES,     4,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};

	static const EGLint context_attributes[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	VC_RECT_T dst_rectangle;
	VC_RECT_T src_rectangle;

	// get default display
	display = eglGetDisplay (EGL_DEFAULT_DISPLAY);
	assert (display != EGL_NO_DISPLAY);

	// initialize display
	result = eglInitialize (display, NULL, NULL);
	assert (result != EGL_FALSE);

	// not sure why we are doing this...
	result = eglSaneChooseConfigBRCM (display, attribute_list, &config, 1, &num);
	assert (result != EGL_FALSE);

	// create rendering context
	context = eglCreateContext (display, config, EGL_NO_CONTEXT, context_attributes);
	assert (context != EGL_NO_CONTEXT);

	// get display size
	success = graphics_get_display_size (LCD, &width, &height);
	assert (success >= 0);

	printf ("  display size: %d x %d\n", width, height);

	dst_rectangle.x = dst_rectangle.y = 0;
	dst_rectangle.width  = width;
	dst_rectangle.height = height;

	src_rectangle.x = src_rectangle.y = 0;
	src_rectangle.width  = width << 16;
	src_rectangle.height = height << 16;

	// i think we are setting up the physical display here
	dispman_display = vc_dispmanx_display_open (LCD);
	dispman_update  = vc_dispmanx_update_start (0);
	dispman_element = vc_dispmanx_element_add (
		dispman_update,
		dispman_display,
		0,
		&dst_rectangle,
		0,
		&src_rectangle,
		DISPMANX_PROTECTION_NONE,
		0,
		0,
		0
	);
	native_window.element = dispman_element;
	native_window.width   = width;
	native_window.height  = height;
	vc_dispmanx_update_submit_sync (dispman_update);

	// create a new surface
	surface = eglCreateWindowSurface (display, config, &native_window, NULL);
	assert (surface != EGL_NO_SURFACE);

	// connect surface to context
	result = eglMakeCurrent (display, surface, surface, context);
	assert (result != EGL_FALSE);

	return 0;
}

/**
 *  Read source file and compile shader as shader_type.
 */
static int compile_shader(const char* source_file, GLuint* shader, GLenum shader_type)
{
	int result;
	FILE* fp = fopen (source_file, "rb");
	if (!fp)
	{
		fprintf (stderr, "could not open shader file [%s]\n", source_file);
		return 1;
	}
	fseek (fp, 0, SEEK_END);
	int file_size = ftell (fp);
	rewind (fp);
	char* source = malloc (file_size);
	memset (source, 0, file_size);

	if ((result = fread (source, 1, file_size, fp)) != file_size)
	{
		fprintf (stderr, "could not read entire shader (only got %d out of %d bytes)\n", result, file_size);
		return 1;
	}
	fclose (fp);

	*shader = glCreateShader (shader_type);
	if (*shader == 0)
		return 1;

	glShaderSource (*shader, 1, (const char**) &source, 0);

	int status;
	glCompileShader (*shader);
	glGetShaderiv (*shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint log_size = 0;
		glGetShaderiv (vertexshader, GL_INFO_LOG_LENGTH, &log_size);

		char error_log[log_size];
		glGetShaderInfoLog (vertexshader, log_size, &log_size, error_log);

		fprintf (stderr, "error compiling shader %s\n%s\n", source_file, error_log);
		return 1;
	}

	free (source);
	return 0;
}

/**
 * Build the shader program to use.
 */
static int build_shader_program ()
{
	// compile shader source
	if (compile_shader (VERTEX_SHADER_FILE, &vertexshader, GL_VERTEX_SHADER) != 0 ||
		compile_shader (FRAGMENT_SHADER_FILE, &fragmentshader, GL_FRAGMENT_SHADER) != 0)
	{
		return 1;
	}
	// create the shader program
	program = glCreateProgram ();
	if (program == 0)
		return 1;

	// attach the shaders
	glAttachShader (program, vertexshader);
	glAttachShader (program, fragmentshader);

	// link and check status to makes sure it went alright
	int linked;
	glLinkProgram  (program);
	glGetProgramiv (program, GL_LINK_STATUS, &linked);
    if (!linked)
	{
		GLint info_length = 0;
		glGetProgramiv (program, GL_INFO_LOG_LENGTH, &info_length);
		if(info_length > 1)
		{
			char* info_log = malloc (sizeof (char) * info_length);
			glGetProgramInfoLog (program, info_length, NULL, info_log);
			fprintf (stderr, "Error linking program:\n%s\n", info_log);
			free (info_log);
		}
		glDeleteProgram (program);
		return 1;
	}
	glUseProgram (program);
	return 0;
}

/**
 *	rpi_ui_create_texture creates a new texture that can be used by widgets.
 */
static GLuint generate_texture (int width, int height, void* data)
{
	GLuint texture;
	glGenTextures   (1, &texture);
	glBindTexture   (GL_TEXTURE_2D, texture);
	glTexImage2D    (GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	return texture;
}

/**
 *	Initialize OpenGL.
 */
static void init_opengl ()
{
	printf ("initializing OpenGL\n");
	if (build_shader_program () != 0)
		exit (1);

	glEnable     (GL_TEXTURE_2D);
	glViewport   (0, 0, width, height);
	glClearColor (0.1f, 0.1f, 0.1f, 1.f);

	// set texture uniform location (only support one active texture at the moment)
	glUniform1i (glGetUniformLocation (program, "tex"), 0);

	// get color uniform location, and initialize the color to white
	color_uniform = glGetUniformLocation (program, "color_in");
	glUniform4f (color_uniform, 1.0, 1.0, 1.0, 1.0);

	// get model matrix uniform location.
	model_uniform = glGetUniformLocation (program, "model");

	// get view matrix uniform location.
	view_uniform = glGetUniformLocation (program, "view");
	glUniformMatrix4fv (view_uniform, 1, GL_FALSE, (GLfloat*) eye);

	// generate our default white texture.
	GLbyte white[4] = {0xff, 0xff, 0xff, 0xff};
	default_texture = generate_texture (1, 1, white);

	// generate vertex buffer for vertices
	GLuint vertex_attrib_location = glGetAttribLocation (program, "vertex");
	glGenBuffers              (1, &vertex_vbo);
	glBindBuffer              (GL_ARRAY_BUFFER, vertex_vbo);
	glBufferData              (GL_ARRAY_BUFFER, 4 * 3 * sizeof (GLfloat), vertex_coords, GL_STATIC_DRAW);
	glEnableVertexAttribArray (vertex_attrib_location);
	glVertexAttribPointer     (vertex_attrib_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// generate vertex buffer for texture coordinates
	GLuint texture_attrib_location = glGetAttribLocation (program, "texture_coords_in");
	glGenBuffers              (1, &texture_vbo);
	glBindBuffer              (GL_ARRAY_BUFFER, texture_vbo);
	glBufferData              (GL_ARRAY_BUFFER, 4 * 2 * sizeof (GLfloat), texture_coords, GL_STATIC_DRAW);
	glEnableVertexAttribArray (texture_attrib_location);
	glVertexAttribPointer     (texture_attrib_location, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

/**
 *  destroy_egl makes sure to unallocate and free all EGL components.
 */
static void destroy_egl ()
{
	eglMakeCurrent    (display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface (display, surface);
	eglDestroyContext (display, context);
	eglTerminate      (display);
}

/**
 *  destroy_opengl deinitializes OpenGL components.
 */
static void destroy_opengl ()
{
	glDeleteTextures (1, &default_texture);
	glDeleteBuffers  (1, &vertex_vbo);
	glDeleteBuffers  (1, &texture_vbo);
	glDeleteShader   (vertexshader);
	glDeleteShader   (fragmentshader);
	glDeleteProgram  (program);
}


int rpi_create_image (GLuint texture, void** egl_image)
{
	glBindTexture (GL_TEXTURE_2D, texture);
	*egl_image = eglCreateImageKHR (
		display,
		context,
		EGL_GL_TEXTURE_2D_KHR,
		(EGLClientBuffer) texture,
		0
	);
	if (*egl_image == EGL_NO_IMAGE_KHR)
	{
		fprintf (stderr, "eglCreateImageKHR failed\n");
		return 1;
	}
	else {
		printf ("new egl image succesfully created\n");
	}
	return 0;
}


int rpi_init_screen ()
{
	if (init_egl () != 0)
		return 1;
	init_opengl ();
	if (rpi_mp_init () != 0)
		return 1;
	return 0;
}


void rpi_render_screen ()
{
	glClear         (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode    (GL_MODELVIEW);
	glLoadIdentity  ();
	// draw widgets !
	for (int i = 0; i < root.nchildren; i ++)
		render_sub_tree (root.children[i], eye);
	eglSwapBuffers  (display, surface);
}


int rpi_deinit_screen ()
{
	destroy_opengl ();
	destroy_egl ();
	return 0;
}

/*
 WIDGETS
*/

int rpi_widget_init (WIDGET widget, WIDGET parent)
{
	// default color to white
	widget->r =
	widget->g =
	widget->b =
	widget->a = 1.f;
	// 0 width and height
	widget->width =
	widget->height = 0.f;
	// no texture
	widget->texture = default_texture;
	widget->egl_image = NULL;
	// default to visible
	widget->visible = 1;
	// eye as model matrix
	memcpy (widget->model, eye, sizeof (GLfloat) * 16);

	memset (widget->text, 0, RPI_MAX_TEXT_LENGTH);
	memset (widget->source, 0, RPI_MAX_TEXT_LENGTH);

	widget->parent = parent;
	// add to node tree
	// no parent, add to root, else look for the parent in the tree
	NODE parent_node = parent == NULL ? &root : find_node (parent);
	new_node (widget, parent_node);

	return 0;
}

void rpi_widget_draw (WIDGET widget)
{
	glBindTexture      (GL_TEXTURE_2D, widget->texture);
	glUniformMatrix4fv (model_uniform, 1, GL_FALSE, (GLfloat*) widget->model);
	glUniform4f        (color_uniform, widget->r, widget->g, widget->b, widget->a);
	glDrawArrays       (GL_TRIANGLE_STRIP, 0, 4);
}

void rpi_widget_image_open (WIDGET widget, const char* path)
{
	rpi_widget_set_source (widget, path);
	// generate if needed a new texture for the widget to use
	if (widget->texture == default_texture)
		widget->texture = generate_texture (1, 1, NULL);
	// create an EGL Image to render to, connected to the texture
	rpi_create_image (widget->texture, &widget->egl_image);
}

int rpi_widget_video_open (WIDGET widget, const char* path, pthread_mutex_t** mut, pthread_cond_t** cond)
{
	rpi_widget_set_source (widget, path);
	// start by opening video source
	int w, h;
	int64_t dur;
	if (rpi_mp_open (path, &w, &h, &dur, RENDER_VIDEO_TO_TEXTURE) != 0)
	{
		fprintf (stderr, "failed to open video source\n");
		return 1;
	}
	if (widget->texture == default_texture)
	{
		// create a new texture to be rendered to
		widget->texture = generate_texture (w, h, NULL);
		// create a new EGL image that will be used for the video components
		if (rpi_create_image (widget->texture, &widget->egl_image) != 0)
		{
			// if we failed to create a target EGL image return
			rpi_mp_stop ();
			return 1;
		}
	}
	// setup media player to render to the image
	rpi_mp_setup_render_buffer (widget->egl_image, mut, cond);
	return 0;
}

void rpi_widget_video_stop (WIDGET widget)
{
	// stop the player
	rpi_mp_stop ();
	// free gpu memory by destroying the image
	if (widget->egl_image != NULL)
		eglDestroyImageKHR (display, (EGLImageKHR) widget->egl_image);
}

void rpi_widget_video_start (WIDGET widget)
{
	rpi_mp_start ();
}

void rpi_widget_video_pause (WIDGET widget) {
	rpi_mp_pause ();
}

int rpi_widget_destroy (WIDGET widget)
{
	// TODO remove among parent's children

	if (widget->egl_image != NULL)
		eglDestroyImageKHR (display, (EGLImageKHR) widget->egl_image);

	if (widget->texture != default_texture)
		glDeleteTextures (1, &widget->texture);
	return 0;
}
