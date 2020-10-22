#version 330

in vec4 inout_positionWorld;
in vec3 inout_normal;
in vec2 inout_texcoord;
in vec2 inout_texcoord2;
in vec3 inout_tangent;
in vec3 inout_bitangent;
in vec3 inout_lightVector;
in vec3 inout_viewVector;

// The pixels final color
out vec4 out_color;

float FAR_PLANE = 250.0;

// Light Definition
struct lightSource
{
	vec4 position;	// World Space
	vec4 diffuse;	// Diffuse color
	vec4 specular;	// Specular color
	vec4 ambient;
	float constantAttenuation, linearAttenuation, quadraticAttenuation;
	float spotCosCutoff, spotExponent;
	vec3 spotDirection;
};

// Material Definition
struct material
{
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

// Solid color or textured
uniform int useColor;
uniform vec4 modelColor;
uniform int blendMode;
uniform sampler2D textureSampler0;
uniform sampler2D textureSampler1;
uniform int useTexture[2];

// Optional normalmapping
uniform int useNormalMapping;
uniform sampler2D normalTexSampler;

// Optional specularmapping
uniform int useSpecularMapping;
uniform sampler2D specularTexSampler;

// With shadowmapping for 4 lights!
uniform samplerCube shadowCubes[4];
uniform int excludeFromLighting;
uniform lightSource lights[4];
float shadowFactors[4];

uniform int numberOfLights;

// TODO: reflect/refract material
// Default Material for now (white -> color preserving)
material frontMaterial = material(
  vec4(0.2, 0.2, 0.2, 1.0),
  vec4(1.0, 1.0, 1.0, 1.0),
  vec4(1.0, 1.0, 1.0, 1.0),
  32.0
);


// Calculate the Attenuation based on the distance to the fragment;
float calculateAttenuation(in int i, in float dist)
{
    return(1.0 / (lights[i].constantAttenuation +
                  lights[i].linearAttenuation * dist +
                  lights[i].quadraticAttenuation * dist * dist));
}


// Reduces light bleeding as descibed in:
// http://http.developer.nvidia.com/GPUGems3/gpugems3_ch08.html
float reduceLightBleeding(in float factor, in float amount)
{
	float tmp = (factor - amount) / (1.0 - amount);
	return clamp(tmp,0.0,1.0);
}


// Calculate the shadow factor with Chebyshev's inequality
float chebyshevUpperBound(samplerCube cube,float distance, vec3 dir)
{
	distance = distance/FAR_PLANE;
	vec2 moments = texture(cube, dir).rg;
	
	// Fragment is before the occluder -> fully lit
	// returning here is slower than adjusting the calculation
	if (distance < moments.x)
		distance = moments.x;

	// Fragment is either in shadow or penumbra.
	float variance = moments.y - (moments.x*moments.x);
	
	// Clamp to some minimum small variance value for numerical stability
	variance = max(variance, 0.00002);
	float diff = moments.x - distance;
	
	// Chebyshev's Inequality will give us the likelyhood of the pixel being lit.
	float upperBound = variance / (variance + diff*diff);
	return reduceLightBleeding(upperBound,0.2);
}

// Calculate the shadow factor for a light
float calculateShadowFactor(int index)
{
	vec3 fragmentToLightWorld = (lights[index].position - inout_positionWorld).xyz;
	return chebyshevUpperBound(shadowCubes[index], length(fragmentToLightWorld), -fragmentToLightWorld);
}


const float cosMinusAngle = 0.0472; // 5 degree
void pointOrSpotLight(in int i, in vec3 N, in vec3 V, in vec3 D, in float shininess, in float spec, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
    vec3 L = normalize(D);
    float dist = length(D);
    float attenuation = calculateAttenuation(i, dist);
    float cosAngleIncidence = dot(N,L);
	float att2 = 1.0;
	
	if (cosAngleIncidence > 0.0)
    {
		if(lights[i].spotCosCutoff != -1.0)
		{
			float spotEffect = dot(normalize(lights[i].spotDirection), -L);
			att2 = smoothstep(lights[i].spotCosCutoff,lights[i].spotCosCutoff+cosMinusAngle,spotEffect);
			attenuation /= pow(spotEffect, lights[i].spotExponent);
		}

		vec3 E = normalize(V);

		// Phong
		vec3 R = reflect(-L, N);
		float pf = pow(max(dot(R,E), 0.0), shininess);

		// Blinn
		// vec3 H = normalize(L + E);
        // float pf = pow(max(dot(N,H), 0.0), shininess);

		shadowFactors[i] = calculateShadowFactor(i);

		diffuse  += lights[i].diffuse  * attenuation * cosAngleIncidence * att2 * shadowFactors[i];
		specular += lights[i].specular * attenuation * pf * shadowFactors[i] * spec;
	}
}


void directionalLight(in int i, in vec3 N, in vec3 V, in vec3 D, in float shininess, in float spec, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
    vec3 L = normalize(lights[i].spotDirection.xyz);
    float cosAngleIncidence = dot(N, L);

    if (cosAngleIncidence > 0.0)
    {   
		vec3 H = normalize(lights[i].position.xyz - V);
        float pf = pow(max(dot(N,H), 0.0), shininess);

        diffuse  += lights[i].diffuse  * cosAngleIncidence;
        specular += lights[i].specular * pf * spec;
    }
}


void calculateLighting(in int numLights, in vec3 N, in vec3 V, in vec3 D, in float shininess, in float spec, inout vec4 ambient, inout vec4 diffuse, inout vec4 specular)
{
    for (int i = 0; i < numLights; i++)
    {
		if (lights[i].position.w == 0.0)
		{
			directionalLight(i, N, V, D, shininess, spec, ambient, diffuse, specular);
		}
		else
		{
			pointOrSpotLight(i, N, V, D, shininess, spec, ambient, diffuse, specular);
		}

		ambient += lights[i].ambient;
    }
}

void main()
{	
	// Color or Texture
	if(useColor > 0)
		frontMaterial.diffuse = modelColor;
	else
	{
		vec4 tex1 = texture(textureSampler0,inout_texcoord);
		vec4 tex2 = texture(textureSampler1,inout_texcoord2) * vec4(useTexture[1]);
		
		if(blendMode == 1)	// Screen
			frontMaterial.diffuse.xyz = 1 - (1 - tex1.xyz) * (1 - tex2.xyz);
		else
		{					// Normal
			frontMaterial.diffuse.xyz = (tex1.xyz * (1-tex2.a)) + (tex2.xyz * tex2.a);
		}

		frontMaterial.diffuse.a = tex1.a;
	}
	
	if(excludeFromLighting > 0)
	{
		out_color = frontMaterial.diffuse;
		return;
	}
	
	vec3 normal = normalize(inout_normal);
	if(useNormalMapping > 0)
	{
		vec3 lookup = texture(normalTexSampler, inout_texcoord).rgb * 2.0 - 1.0;
		normal =  (lookup.x * inout_tangent) 
				+ (lookup.y * inout_bitangent)
				+ (lookup.z * normal);
		normal = normalize(normal);
	}

	float specularAmount = 0.0;
	if(useSpecularMapping > 0)
	{
		specularAmount = texture(specularTexSampler, inout_texcoord).r;
	}
	
	// Front light
	vec4 ambientColor = vec4(0.0);
	vec4 diffuseColor = vec4(0.0);
	vec4 specularColor = vec4(0.0);
	
	calculateLighting(numberOfLights, normal, inout_viewVector, inout_lightVector, frontMaterial.shininess, specularAmount, ambientColor, diffuseColor, specularColor);
	vec4 finalColor  = (ambientColor * frontMaterial.diffuse) + (diffuseColor * frontMaterial.diffuse) + (specularColor * frontMaterial.specular);

	out_color = clamp(finalColor,0.0,1.0);
	out_color.a = frontMaterial.diffuse.a;
}