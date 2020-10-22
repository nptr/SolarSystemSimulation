#version 330
in vec2 inout_texcoord;
out vec4 out_color;

uniform sampler2D textureSampler0;
uniform sampler2D textureSampler1;

void main()
{	
	vec4 dst = texture2D(textureSampler0, inout_texcoord);
	vec4 src = texture2D(textureSampler1, inout_texcoord);

    vec4 linearColor = clamp((src + dst) - (src * dst), 0.0, 1.0);
    
    // manual gamma correction
    //out_color.rgb = pow(linearColor.rgb, 1.0 / vec3(2.2,2.2,2.2));
    
	out_color.rgb = linearColor.rgb;
	out_color.a = 1.0;
}