#version 330

// Untransformed Vertex Inputs (Model Space)
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

out vec2 inout_texcoord;

void main()
{
	// Input data is already in [-1..1] range
    gl_Position = vec4(in_position,1);
	inout_texcoord = in_texcoord;
}