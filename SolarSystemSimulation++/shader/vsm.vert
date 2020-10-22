#version 330

layout(location = 0) in vec3 in_position;

out vec4 vPositionEye;

uniform mat4 cameraToShadowView;
uniform mat4 cameraToShadowProjector;
uniform mat4 model;
	
void main() {
	gl_Position = cameraToShadowProjector * model * vec4(in_position, 1.0);
	vPositionEye  = cameraToShadowView * model * vec4(in_position, 1.0);
};