#version 450

out vec3 color;

in vec2 UV;


in vec3 normal;
in vec3 lightDirection;

in vec3 position_worldspace;
in vec3 normal_cameraspace;
in vec3 lightDirection_cameraspace;

uniform mat4 MVP;
uniform sampler2D cubeTexture;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float lightIntensity;

void main() {
    vec3 n = normalize(normal_cameraspace);
    vec3 l = normalize(lightDirection_cameraspace);
    float distanceLight = length(position_worldspace - lightPosition);
    float cosTheta = clamp(dot(n, l), 0, 1 );
    color = texture(cubeTexture, UV).rgb * lightColor * lightIntensity * cosTheta / (distanceLight * distanceLight);
}