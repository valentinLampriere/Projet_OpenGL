#version 450

out vec4 color;

in vec2 UV;


in vec3 Normal_cameraspace;
in vec3 LightDirection_cameraspace;

uniform mat4 MVP;
uniform sampler2D cubeTexture;

void main() {
    vec3 n = normalize( Normal_cameraspace );
    vec3 l = normalize( LightDirection_cameraspace );

    color = vec4(texture(cubeTexture, UV).rgb, 1);
}