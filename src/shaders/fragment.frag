/**
 *   File: fragment.frag
 *   Description: Fragment shader source.
 */
precision highp float;

uniform sampler2D tex;

varying vec2 texture_coords;
varying vec4 color;

void main ()
{
	gl_FragColor = texture2D (tex, texture_coords) * color;
}
