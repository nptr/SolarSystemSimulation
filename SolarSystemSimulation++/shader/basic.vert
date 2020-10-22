/* 
 * Simple Vertex Shader - useful for drawing unlit textured objects
 * Transfroms vertices to world space and passes texture coord
 * to the fragment shader.
 */

#version 330

// Untransformed Inputs (Model Space)
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_texcoord;

// Transformation Matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 inout_texcoord;

void main()
{
	inout_texcoord = in_texcoord;
	gl_Position = proj * view * model * vec4(in_position, 1.0f);
}