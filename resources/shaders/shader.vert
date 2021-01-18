#version 450

layout(location = 0) in vec3 vertexPosition;

layout(location = 1) in vec2 vertexUV;

layout(location = 2) in vec2 vertexNormal;

uniform mat4 MVP;

out vec2 UV;
out vec3 Normal_cameraspace;
out vec3 LightDirection_cameraspace;


void main() {
	vec3 lightDirection = vec3(0, -1, 0);
	gl_Position = MVP * vec4(vertexPosition, 1.0);
	UV = vertexUV;
	LightDirection_cameraspace = lightDirection;
}