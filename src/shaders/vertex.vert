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
uniform mat4 modelview;
// modelview matrix of the room the component is part, i.e. modelview of parent component
uniform mat4 room;

// color to send to fragment shader
varying vec4 color;
// texture coordinates to send to the fragment shader
varying vec2 texture_coords;

void main ()
{
	gl_Position = modelview * vec4 (vertex, 1.0);
	texture_coords = texture_coords_in;
	color = color_in;
}
