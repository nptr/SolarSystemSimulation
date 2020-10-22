#version 330

// Untransformed Vertex Inputs (Model Space)
layout(location = 0) in vec3 in_position;

// Output
out vec3 in_texcoord;

// Transformation Matrices
uniform mat4 model, view, proj;

void main()
{
	mat4 invProj = inverse(proj);
	mat3 invView = transpose(mat3(view));
	vec3 unproj = (invProj * vec4(in_position,1.0)).xyz;
	in_texcoord = invView * unproj;
	gl_Position = vec4(in_position,1.0);
}