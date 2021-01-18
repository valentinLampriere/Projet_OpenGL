#version 450

layout(location = 0) in vec3 cubeVertexPosition_modelspace;

layout(location = 1) in vec2 vertexUV;

uniform mat4 MVP;

out vec2 UV;

void main() {
	gl_Position = MVP * vec4(cubeVertexPosition_modelspace, 1.0);
	UV = vertexUV;
}