#version 330

uniform sampler2D fontTexture;

in vec2 inout_texcoord;
out vec4 out_color;

void main()
{
	// Use only texture alpha
	out_color = vec4(0.9,0.9,0.9,texture(fontTexture, inout_texcoord).a);
}