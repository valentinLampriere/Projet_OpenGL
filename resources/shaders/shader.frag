#version 450

out vec4 color;

in vec3 fragmentColor;

void main() {
    //color = vec4(1, 0, 0, 1);
    color = vec4(fragmentColor, 1);
}