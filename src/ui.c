#include "rpi_ui.h"
#include "rpi_ui_widget.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>


#define LCD 0
#define FRAGMENT_SHADER_FILE "src/shaders/fragment.frag"
#define VERTEX_SHADER_FILE "src/shaders/vertex.vert"
#define N_CHILDREN_ALLOC 8


typedef rpi_widget* WIDGET;

/**
 *  Keep a tree of the widgets in the window.
 *  This struct represents a node in the tree.
 */
struct widget_node
{
	WIDGET widget;
	int    nchildren;
	struct widget_node** children;
};
typedef struct widget_node* NODE;

/* root node of the widgets */
static struct widget_node root;


static uint32_t     width,
                    height;
static EGLDisplay   display;
static EGLSurface   surface;
static EGLContext   context;

static GLuint       program,
                    vertexshader,
                    fragmentshader,
                    color_uniform,
                    vertex_vbo,
                    texture_vbo,
                    modelview_uniform;


/* Default texture coordinates */
static GLfloat texture_coords[6 * 2] =
{
	0.0, 1.0,
	0.0, 0.0,
	1.0, 0.0,
	1.0, 0.0,
	1.0, 1.0,
	0.0, 1.0
};

/* Default vertex coordinates. */
static GLfloat vertex_coords[6 * 3] =
{
	-1.0, -1.0,  0.0,
	-1.0,  1.0,  0.0,
	 1.0,  1.0,  0.0,
	 1.0,  1.0,  0.0,
	 1.0, -1.0,  0.0,
	-1.0, -1.0,  0.0
};


/**
 *  Render widget connected to node and continue down its children.
 *  Depth first style.
*/
static void render_sub_tree (NODE node)
{
	int i = 0;

	// TODO set up context in shader

 	// first draw all children
	for (i = 0; i < node->nchildren; i ++)
		rpi_widget_draw (node->children[i]->widget);

	// then
	for (i = 0; i < node->nchildren; i ++)
		render_sub_tree (node->children[i]);
}

/**
 *  Search the widget tree for the node corresponding to the widget.
 */
static NODE find_node (WIDGET widget)
{
	int found = 0;
	while (!found)
	{
		found = 1; // TODO implement
	}
	return NULL;
}

/**
 *  Append the node to the parents children.
 */
static void append_to_children (NODE node, NODE child)
{
	if ((node->nchildren % N_CHILDREN_ALLOC) == 0)
	{
		// we need to allocate a new larger buffer for children.
		NODE* children = node->children;
		node->children = malloc (sizeof (NODE) * (node->nchildren + N_CHILDREN_ALLOC));
		memcpy (node->children, children, node->nchildren);
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
	new_node->widget      = widget;
	new_node->children    = malloc (sizeof (NODE) * N_CHILDREN_ALLOC);
	// append to parent
	append_to_children(parent, new_node);
	return new_node;
}


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
	widget->texture = 0;
	// eye matrix
	widget->model[3][3] = 1.f;

	memset (widget->text, 0, RPI_MAX_TEXT_LENGTH);
	memset (widget->source, 0, RPI_MAX_TEXT_LENGTH);

	widget->parent = parent;
	// add to node tree
	// no parent, add to root
	NODE parent_node = &root;
	if (parent != NULL)
	{
		// traverse tree looking for the node representing the parent
		parent_node = find_node (parent);
	}
	// add the widget to the tree
	new_node (widget, parent_node);

	return 0;
}


/**
 *  Read source file and compile shader.
 */
static int compile_shader(const char* source_file, GLuint* shader, GLuint shader_type)
{
	int result;
	FILE* fp = fopen (source_file, "rb");
	if (!fp)
	{
		fprintf (stderr, "could not open vertex shader file\n");
		return 1;
	}
	fseek (fp, 0, SEEK_END);
	int file_size = ftell (fp);
	rewind (fp);
	char* source = (char*) malloc (file_size);
	memset (source, 0, file_size);

	if ((result = fread (source, 1, file_size - 1, fp)) != file_size - 1)
	{
		fprintf (stderr, "could not read entire vertex shader (only got %d out of %d bytes)\n", result, file_size);
		return 1;
	}
	fclose (fp);

	*shader = glCreateShader (shader_type);
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
 * Compile shaders and create shader program.
 */
static int build_shader_program ()
{
	// compile shader source
	if (compile_shader (VERTEX_SHADER_FILE, &vertexshader, GL_VERTEX_SHADER) != 0 ||
		compile_shader (FRAGMENT_SHADER_FILE, &fragmentshader, GL_FRAGMENT_SHADER) != 0)
	{
		return 1;
	}
	// create the shader program and attach the shaders
	program = glCreateProgram ();
	glAttachShader (program, fragmentshader);
	glAttachShader (program, vertexshader);
	glLinkProgram  (program);
	glUseProgram   (program);
	return 0;
}


static int init_egl ()
{
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
		EGL_SURFACE_TYPE,
		EGL_WINDOW_BIT,
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
	context = eglCreateContext (display, config, EGL_NO_CONTEXT, NULL);
	assert (context != EGL_NO_CONTEXT);

	// get display size
	success = graphics_get_display_size (LCD, &width, &height);
	assert (success >= 0);

	printf ("display size: %d x %d\n", width, height);

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


static void init_opengl ()
{
	build_shader_program ();

	glEnable        (GL_TEXTURE_2D);

	glMatrixMode    (GL_PROJECTION);
	glViewport      (0, 0, width, height);
	glOrthof        (0, 1, 0, 1, -10, 10);
	glClearColor    (0.1f, 0.2f, 0.2f, 1.f);

	// generate vertex buffer for vertices and texture coords
	glGenBuffers	(1, &vertex_vbo);
	glGenBuffers 	(1, &texture_vbo);

	glBindBuffer 	(GL_ARRAY_BUFFER, texture_vbo);
	glBufferData	(GL_ARRAY_BUFFER, 6 * 2 * sizeof (GLfloat), texture_coords, GL_STATIC_DRAW);

	glBindBuffer (GL_ARRAY_BUFFER, vertex_vbo);
	glBufferData (GL_ARRAY_BUFFER, 6 * 3 * sizeof (GLfloat), vertex_coords, GL_STATIC_DRAW);
	GLuint vertex_attrib_location = glGetAttribLocation (program, "vertex");
	glEnableVertexAttribArray (vertex_attrib_location);
	glVertexAttribPointer (vertex_attrib_location, 3, GL_FLOAT, 0, 0, 0);

	glBindBuffer (GL_ARRAY_BUFFER, texture_vbo);
	glBufferData (GL_ARRAY_BUFFER, 6 * 2 * sizeof (GLfloat), texture_coords, GL_STATIC_DRAW);
	GLuint texture_attrib_location = glGetAttribLocation (program, "texture_coords_in");
	glEnableVertexAttribArray (texture_attrib_location);
	glVertexAttribPointer (texture_attrib_location, 2, GL_FLOAT, 0, 0, 0);

	glUniform1i (glGetUniformLocation (program, "tex"), 0);

	color_uniform = glGetUniformLocation (program, "color_in");
	glUniform4f (color_uniform, 1.0, 1.0, 1.0, 1.0);

	modelview_uniform = glGetUniformLocation (program, "modelview");
}


static void destroy_egl ()
{
	eglMakeCurrent      (display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroySurface   (display, surface);
	eglDestroyContext   (display, context);
	eglTerminate        (display);
}


static void destroy_opengl ()
{
	glDeleteShader  (vertexshader);
	glDeleteShader  (fragmentshader);
	glDeleteProgram (program);
}


int rpi_init_screen ()
{
	if (init_egl() != 0)
		return 1;
	init_opengl ();
	return 0;
}


int rpi_deinit_screen ()
{
	destroy_opengl ();
	destroy_egl ();
	return 0;
}


void rpi_render_screen ()
{
	glMatrixMode    (GL_MODELVIEW);
	glClear         (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity  ();
	// draw widgets !
	render_sub_tree (&root);
	eglSwapBuffers  (display, surface);
}


int rpi_create_image (GLuint texture, void** egl_image)
{
	glBindTexture (GL_TEXTURE_2D, texture);
	*egl_image = eglCreateImageKHR
	(
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
	return 0;
}
