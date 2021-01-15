#version 450

out vec4 color;

in vec2 UV;
uniform sampler2D myTextureSampler;

void main() {
    color = vec4(texture(myTextureSampler, UV).rgb, 1);
}