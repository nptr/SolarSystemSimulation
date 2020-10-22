/* 
 * Gaussian Blur Shader
 * Kernelsize: 9
 * Sigma: 3
 */

#version 330

uniform vec2 ScaleU;
uniform sampler2D textureSampler0;

in vec2 inout_texcoord;
out vec4 out_color;


vec4 blur9(sampler2D image, vec2 uv, vec2 resolution, vec2 direction)
{
  vec4 color = vec4(0.0);

  vec2 off1 = vec2(1.3846153846) * direction;
  vec2 off2 = vec2(3.2307692308) * direction;
  color += texture2D(image, uv) * 0.2270270270;
  color += texture2D(image, uv + (off1 / resolution)) * 0.3162162162;
  color += texture2D(image, uv - (off1 / resolution)) * 0.3162162162;
  color += texture2D(image, uv + (off2 / resolution)) * 0.0702702703;
  color += texture2D(image, uv - (off2 / resolution)) * 0.0702702703;

  return color;
}


vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction)
{
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.411764705882353) * direction;
  vec2 off2 = vec2(3.2941176470588234) * direction;
  vec2 off3 = vec2(5.176470588235294) * direction;
  color += texture2D(image, uv) * 0.1964825501511404;
  color += texture2D(image, uv + (off1 / resolution)) * 0.2969069646728344;
  color += texture2D(image, uv - (off1 / resolution)) * 0.2969069646728344;
  color += texture2D(image, uv + (off2 / resolution)) * 0.09447039785044732;
  color += texture2D(image, uv - (off2 / resolution)) * 0.09447039785044732;
  color += texture2D(image, uv + (off3 / resolution)) * 0.010381362401148057;
  color += texture2D(image, uv - (off3 / resolution)) * 0.010381362401148057;

  return color;
}


void main()
{
	vec4 color = vec4(0.0);
	color += texture2D( textureSampler0, inout_texcoord + vec2( -4.0*ScaleU.x, -4.0*ScaleU.y ) ) * 0.063327;
	color += texture2D( textureSampler0, inout_texcoord + vec2( -3.0*ScaleU.x, -3.0*ScaleU.y ) ) * 0.093095;
	color += texture2D( textureSampler0, inout_texcoord + vec2( -2.0*ScaleU.x, -2.0*ScaleU.y ) ) * 0.122589;
	color += texture2D( textureSampler0, inout_texcoord + vec2( -1.0*ScaleU.x, -1.0*ScaleU.y ) ) * 0.144599;
	color += texture2D( textureSampler0, inout_texcoord + vec2( 0.0 , 0.0) )						* 0.152781;
	color += texture2D( textureSampler0, inout_texcoord + vec2( 1.0*ScaleU.x,  1.0*ScaleU.y ) ) * 0.144599;
	color += texture2D( textureSampler0, inout_texcoord + vec2( 2.0*ScaleU.x,  2.0*ScaleU.y ) ) * 0.122589;
	color += texture2D( textureSampler0, inout_texcoord + vec2( 3.0*ScaleU.x,  3.0*ScaleU.y ) ) * 0.093095;
	color += texture2D( textureSampler0, inout_texcoord + vec2( 4.0*ScaleU.x,  4.0*ScaleU.y ) ) * 0.063327;
	out_color = color;
}