/* 
 * This shader removes the translation part of the
 * view matrix in order to make the rendered objects
 * seem very far away and unreachable.
 * It also misuses the normalbuffer for colors
 */

#version 330

// Untransformed Inputs (Model Space)
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

// Transformation Matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 inout_color;

void main()
{
	inout_color = in_normal;
    mat4 vv = view;
    vv[3][0] = 0.0f;
    vv[3][1] = 0.0f;
    vv[3][2] = 0.0f;
	vec4 pos = proj * vv * model * vec4(in_position, 1.0f);
    gl_Position = pos;
}