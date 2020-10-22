#version 330

in vec3 inout_color;
out vec4 out_color;

void main()
{
	// darken the stars to only let the brightest of
	// shine through
	out_color = vec4(inout_color*0.05, 1.0);
}