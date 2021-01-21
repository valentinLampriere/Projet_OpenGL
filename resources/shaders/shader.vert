#version 450

layout(location = 0) in vec3 vertexPosition_modelspace;

layout(location = 1) in vec2 vertexUV_modelspace;

layout(location = 2) in vec3 vertexNormal_modelspace;

layout(location = 3) in vec3 vertexColor_modelspace;

layout(location = 4) in vec3 vertexTangent_modelspace;

layout(location = 5) in vec3 vertexBitangent_modelspace;

out vec2 UV;
out vec3 normal;
out vec3 lightDirection;
out vec3 position_worldspace;
out vec3 normal_cameraspace;
out vec3 lightDirection_cameraspace;
out vec3 eyeDirection_cameraspace;
out vec3 vertexColor;

out vec3 vertexNormal_cameraspace;
out vec3 vertexTangent_cameraspace;
out vec3 vertexBitangent_cameraspace;
out vec3 lightDirection_tangentspace;
out vec3 eyeDirection_tangentspace;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 V;
uniform mat3 MV3x3;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float lightIntensity;

void main() {
	vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
	eyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;
	
	vec3 lightPosition_cameraspace = ( V * vec4(lightPosition,1)).xyz;
	lightDirection_cameraspace = lightPosition_cameraspace + eyeDirection_cameraspace;

	position_worldspace = (M * vec4(vertexPosition_modelspace, 1)).xyz;
	normal_cameraspace = (V * M * vec4(vertexNormal_modelspace, 0)).xyz;
 
	if (length(vertexTangent_modelspace) > 0) {
		vertexNormal_cameraspace = MV3x3 * normalize(vertexNormal_modelspace);
		vertexTangent_cameraspace = MV3x3 * normalize(vertexTangent_modelspace);
		vertexBitangent_cameraspace = MV3x3 * normalize(vertexBitangent_modelspace);
	}

	lightDirection = normalize(lightPosition - gl_Position.xyz);
	gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);
	UV = vertexUV_modelspace;
	normal = vertexNormal_modelspace;
	vertexColor = vertexColor_modelspace;

	mat3 TBN = transpose(mat3(
        vertexTangent_cameraspace,
        vertexBitangent_cameraspace,
        vertexNormal_cameraspace
    ));

	lightDirection_tangentspace = TBN * lightDirection_cameraspace;
    eyeDirection_tangentspace =  TBN * eyeDirection_cameraspace;
}