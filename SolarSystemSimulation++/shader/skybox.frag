#version 330

uniform samplerCube skyboxTex;
uniform float brightness;

in vec3 in_texcoord;
out vec4 out_color;

void main()
{
	vec4 tmpColor = texture(skyboxTex, in_texcoord);

	// desaturate and darken the texture
	out_color.rgb = pow(tmpColor.rgb, 1.0 / vec3(2.2)) * brightness; //0.015;
	
	// add a slight touch of blue to make the
	// scene more realistic.
	out_color.b += 0.0015;
	out_color.a = 1.0;
}