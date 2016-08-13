attribute vec3 vertex;

uniform vec2 texture_coords_in;
uniform mat4 modelview;
uniform vec4 color_in;

varying vec4 color;
varying vec2 texture_coords;

void main ()
{
	gl_Position = modelview * vec4 (vertex, 1.0);
	texture_coords = texture_coords_in;
	color = color_in;
}
