#version 450

out vec4 color;

in vec2 UV;

uniform mat4 MVP;
uniform sampler2D cubeTexture;

void main() {
    color = vec4(texture(cubeTexture, UV).rgb, 1);
}