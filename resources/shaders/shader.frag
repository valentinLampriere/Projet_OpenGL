#version 450

out vec4 color;

in vec2 UV;


in vec3 normal;
in vec3 lightDirection;

in vec3 position_worldspace;
in vec3 normal_cameraspace;
in vec3 lightDirection_cameraspace;
in vec3 eyeDirection_cameraspace;

uniform mat4 MVP;
uniform sampler2D cubeTexture;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float lightIntensity;

void main() {
    vec3 n = normalize(normal_cameraspace);
    vec3 l = normalize(lightDirection_cameraspace);
    
    vec3 E = normalize(eyeDirection_cameraspace);
    vec3 R = reflect(-l,n);

    vec3 materialDiffuseColor = texture(cubeTexture, UV).rgb;
    vec3 materialAmbientColor = vec3(0.15,0.15,0.15) * materialDiffuseColor;
    vec3 materialSpecularColor = vec3(0.3,0.3,0.3);

    float distanceLight = length(position_worldspace - lightPosition);
    float cosTheta = clamp(dot(n, l), 0, 1 );
    
    float cosAlpha = clamp( dot( E,R ), 0,1 );

    color = vec4(
        materialAmbientColor +
        materialDiffuseColor * lightColor * lightIntensity * cosTheta / (distanceLight * distanceLight) +
        materialSpecularColor * lightColor * lightIntensity * pow(cosAlpha,5) / (distanceLight * distanceLight), 0.3);
}