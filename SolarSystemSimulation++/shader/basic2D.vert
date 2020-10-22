#version 330

/*
 * Basic vertex shader
 * We just transform the model
 * No shading, lighting, normalmapping, ...
 */

// Untransformed Vertex Inputs (Model Space)
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

out vec2 inout_texcoord;

// Transformation Matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	inout_texcoord = in_texcoord;
	gl_Position = vec4(in_position, 1.0);
}