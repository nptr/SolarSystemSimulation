#version 330

// Untransformed Vertex Inputs (model space)
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;
layout(location = 3) in vec3 in_tangent;
layout(location = 4) in vec3 in_bitangent;

// Transformed Outputs
out vec4 inout_positionWorld;
out vec3 inout_normal;
out vec2 inout_texcoord;
out vec2 inout_texcoord2;
out vec3 inout_tangent;
out vec3 inout_bitangent;
out vec3 inout_lightVector;
out vec3 inout_viewVector;

// Transformation Matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat3 texTransform[2];

// Light Definition
struct lightSource
{
	vec4 position;  // model space
	vec4 diffuse;   // diffuse color
	vec4 specular;  // specular color
	vec4 ambient;
	float constantAttenuation, linearAttenuation, quadraticAttenuation;
	float spotCosCutoff, spotExponent;
	vec3 spotDirection;
};

uniform lightSource lights[4];
uniform vec4 viewPosition;

// Multiplication of Normals:
// transform(inverse(M)) for non orthhogonal matrices
// M for orthogonal matrics (Only R,Ts or uniform S)

// Strange Lighting with these...why?
//vNormalEye = normalize(view * transpose(inverse(model)) * vec4(in_normal,   0.0f));
//vNormalWorld = normalize(transpose(inverse(model)) * vec4(in_normal,   0.0f));

void main()
{
	inout_positionWorld = model * vec4(in_position, 1.0);

	// Basics - transform model and pass on texture coordinates
	inout_texcoord = (texTransform[0] * vec3(in_texcoord,1.0)).xy;
	inout_texcoord2 = (texTransform[1] * vec3(in_texcoord,1.0)).xy;
	gl_Position = proj * view * model * vec4(in_position, 1.0);
	
	// Calc the vectors for shading
	vec3 viewVectorWorldSpace = viewPosition.xyz - inout_positionWorld.xyz;
	vec3 lightVectorWorldSpace = lights[0].position.xyz - inout_positionWorld.xyz;
	
	// Prepare Lighting & Normal Mapping
	inout_normal = normalize(vec3(model * vec4(in_normal, 0.0)));
	inout_tangent = normalize(vec3(model * vec4(in_tangent, 0.0)));
	inout_bitangent = normalize(vec3(model * vec4(in_bitangent, 0.0)));
	inout_viewVector = viewVectorWorldSpace;
	inout_lightVector = lightVectorWorldSpace;
}