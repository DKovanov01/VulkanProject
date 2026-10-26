#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_LIGHTS 4
layout (location = 0) in vec3 vertNormal;
layout (location = 1) in vec3 eyeDir;
layout (location = 2) in vec2 fragTexCoords;
layout (location = 3) in vec3 lightDir[MAX_LIGHTS];


layout(set = 0, binding = 1) uniform LightsUBO {
 vec4 pos[MAX_LIGHTS];
 vec4 diffuse[MAX_LIGHTS];
 vec4 specular[MAX_LIGHTS];
 vec4 ambient;
 uint numLights;
} lights;

layout (location = 0) out vec4 fragColor;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

void main() { 
	vec3 reflection[MAX_LIGHTS];
	float spec[MAX_LIGHTS];
	float diff[MAX_LIGHTS];
	vec4 ka = lights.ambient;
	vec4 kt = texture(texSampler,fragTexCoords); 
	vec4 phongResult = vec4(0.0,0.0,0.0,0.0);


	for(uint i =0; i < lights.numLights; i++){
		vec4 ks = lights.specular[i];
		vec4 kd = lights.diffuse[i]; 
		diff[i] = max(dot(vertNormal, lightDir[i]), 0.0);
		reflection[i] = normalize(reflect(-lightDir[i], vertNormal));
		spec[i] = max(dot(eyeDir, reflection[i]), 0.0);
		spec[i] = pow(spec[i],14.0);
		phongResult +=  (ka*kt) + (diff[i] * kd * kt) + (spec[i] * ks * kt);
	}
	fragColor =  phongResult;
} 

