#version 330

in vec4 vPositionEye;
out vec4 out_color;

void main() 
{
	// Divide length by far plane to get a depth value between 0 and 1
	float depth = length(vec3(vPositionEye)) / 250.0;

	float moment1 = depth;
	float moment2 = depth * depth;

	float dx = dFdx(depth);
	float dy = dFdy(depth);
	moment2 += 0.25*(dx*dx+dy*dy);

	out_color = vec4(moment1, moment2, 0.0, 0.0);
};