/* 
 * Gaussian Blur Shader - 7x1
 */

#version 330

uniform vec3 ScaleU;
uniform samplerCube textureSource;

in vec3 in_texcoord;
out vec4 out_color;

void main()
{
	vec4 color = vec4(0.0);
	
	color += texture( textureSource, in_texcoord + vec3( -3.0*ScaleU.x, -3.0*ScaleU.y, -3.0*ScaleU.z )) * 0.015625;
	color += texture( textureSource, in_texcoord + vec3( -2.0*ScaleU.x, -2.0*ScaleU.y, -2.0*ScaleU.z )) * 0.09375;
	color += texture( textureSource, in_texcoord + vec3( -1.0*ScaleU.x, -1.0*ScaleU.y, -1.0*ScaleU.z )) * 0.234375;
	color += texture( textureSource, in_texcoord ) * 0.3125;
	color += texture( textureSource, in_texcoord + vec3( 1.0*ScaleU.x,  1.0*ScaleU.y, 1.0*ScaleU.z)) * 0.234375;
	color += texture( textureSource, in_texcoord + vec3( 2.0*ScaleU.x,  2.0*ScaleU.y, 2.0*ScaleU.z)) * 0.09375;
	color += texture( textureSource, in_texcoord + vec3( 3.0*ScaleU.x,  3.0*ScaleU.y, 3.0*ScaleU.z)) * 0.015625;
	//color = texture( textureSource, in_texcoord);
	out_color = color;
}