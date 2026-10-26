#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform GlobalLightingUBO {
    vec4 pos;
    vec4 diffuse;
    vec4 specular;
    vec4 ambient;
} glights;

layout (location = 0) in vec3 vertNormal;
layout (location = 1) in vec3 lightDir;
layout (location = 2) in vec3 eyeDir; 
layout (location = 3) in vec2 fragTexCoords;

layout (location = 0) out vec4 fragColor;

layout(binding = 2) uniform sampler2D texSampler;


void main() { 
	vec4 ks = glights.specular;    
	vec4 kd = glights.diffuse;    
	vec4 kt = texture(texSampler,fragTexCoords);
	vec4 ka = glights.ambient;
	
	float diff = max(dot(vertNormal, lightDir), 0.0);

	/// Reflection is based incedent beam of light which means a vector 
	/// from the light source not the direction to the light source. 
	/// Put a minus sign on light dir to turn it in the opposite direction. 
	vec3 reflection = normalize(reflect(-lightDir, vertNormal));

	float spec = max(dot(eyeDir, reflection), 0.0);
	spec = pow(spec,14.0);
	vec4 ambient = ka * kt;
	vec4 diffuse = diff * kd * kt;
	vec4 specular = spec * ks + kt;
	fragColor =  ambient + diffuse + specular;
} 

