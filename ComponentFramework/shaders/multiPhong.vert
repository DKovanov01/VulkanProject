#version 450
#extension GL_ARB_separate_shader_objects : enable
#define MAX_LIGHTS 4

layout (location = 0) in  vec4 vVertex;
layout (location = 1) in  vec4 vNormal;
layout (location = 2) in  vec2 texCoords;

layout(set 0, binding = 0) uniform CameraUBO {
    mat4 projectionMatrix;
	mat4 viewMatrix;
} camera;

layout(set 0, binding = 1) uniform LightsUBO {
 vec4 pos[MAX_LIGHTS]; // [MAX_LIGHTS] -> it must be given a constant, and we have to decide how many lights we have beforehand.
 vec4 diffuse[MAX_LIGHTS];
 vec4 specular[MAX_LIGHTS];
 vec4 ambient;
 uint numLights;
} lights;

layout(push_constant) uniform Push { // this is kinda like a structure
	mat4 modelMatrix; // give it some stuff
	mat3x4 normalMatrix; 
	vec4 pad;
} push; // the object/variable is called push, it's defined in the {} - so down below we could say push.modelMatrix, and so we have access to the modelMatrix

layout (location = 0) out vec3 vertNormal;
layout (location = 1) out vec3 eyeDir;
layout (location = 2) out vec2 fragTexCoords;
layout (location = 3) out vec3 lightDir[MAX_LIGHTS];


void main() {
	fragTexCoords = texCoords;
	//mat3 normalMatrix = mat3(transpose(inverse(push.modelMatrix))); // we're still doing that :3 Stop being dumb, delete this, fix it; Leave no trace
	mat3 normalMatrix = mat3(push.normalMatrix);
	// use the stuff from uniform Push on line 14 instead

	vertNormal = normalize(normalMatrix * vNormal.xyz); /// Rotate the normal to the correct orientation // the Normal might give us some grief. Typecast back to a 3x3 matrix?
	vec3 vertPos = vec3(camera.viewMatrix * push.modelMatrix * vVertex); /// This is the position of the vertex from the origin
	vec3 vertDir = normalize(vertPos);
	eyeDir = -vertDir;
	/// Light position from the point-of-view of each vertex
	vec3 lightLocFromVertex[MAX_LIGHTS];
	for(int i =0; i < lights.numLights; i++){
		lightLocFromVertex[i] = vec3(lights.pos[i]) - vertPos; 
		lightDir[i] = normalize(lightLocFromVertex[i]); /// Create the light direction.
	}
	
	gl_Position =  camera.projectionMatrix * camera.viewMatrix * push.modelMatrix * vVertex; 
}
