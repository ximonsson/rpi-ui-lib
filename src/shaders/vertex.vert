/**
 *   File: vertex.vert
 *   Description: Vertex shader source.
 */

// input vertex coordinate.
attribute vec3 vertex;

// input color
uniform vec4 color_in;
// input texture coordinates
uniform vec2 texture_coords_in;
// modelview matrix of the component we are rendering
uniform mat4 model;
// modelview matrix of the room the component is rendered in,
// i.e. modelview of parent component
uniform mat4 view;

// color to send to fragment shader
varying vec4 color;
// texture coordinates to send to the fragment shader
varying vec2 texture_coords;

// projection matrix to map coordinates 0,0 to the bottom left, and 1,1 to the top right
mat4 projection = mat4 (
	 2.0,  0.0,  0.0,  0.0,
	 0.0,  2.0,  0.0,  0.0,
	 0.0,  0.0,  0.0,  0.0,
	-1.0, -1.0,  0.0,  1.0
);

void main ()
{
	gl_Position = projection * /* view * */  model * vec4 (vertex, 1.0);
	texture_coords = texture_coords_in;
	color = color_in;
}
