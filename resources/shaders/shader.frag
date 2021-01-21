#version 450

out vec4 color;

in vec2 UV;


in vec3 normal;
in vec3 lightDirection;

in vec3 position_worldspace;
in vec3 normal_cameraspace;
in vec3 lightDirection_cameraspace;
in vec3 eyeDirection_cameraspace;
in vec3 vertexColor;

in vec3 lightDirection_tangentspace;
in vec3 eyeDirection_tangentspace;

uniform mat4 MVP;
uniform sampler2D cubeTexture;
uniform sampler2D normalTexture;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float lightIntensity;

void main() {



    vec3 n = normalize(normal_cameraspace);
    vec3 l = normalize(lightDirection_cameraspace);
    
    vec3 E = normalize(eyeDirection_cameraspace);

    
    if (length(eyeDirection_tangentspace) > 0) {
        vec3 textureNormal_tangentspace = normalize(texture( normalTexture, UV ).rgb*2.0 - 1.0);

        n = normalize(textureNormal_tangentspace);
        l = normalize(lightDirection_tangentspace);
    
        E = normalize(eyeDirection_cameraspace);
    }
    vec3 R = reflect(-l,n);

    vec3 materialDiffuseColor = texture(cubeTexture, UV).rgb + vertexColor;
    vec3 materialAmbientColor = vec3(0.15,0.15,0.15) * materialDiffuseColor;
    vec3 materialSpecularColor = vec3(0.3,0.3,0.3);

    float distanceLight = length(position_worldspace - lightPosition);
    float cosTheta = clamp(dot(n, l), 0, 1 );
    
    float cosAlpha = clamp( dot( E,R ), 0,1 );

    color = vec4(
        materialAmbientColor +
        materialDiffuseColor * lightColor * lightIntensity * cosTheta / (distanceLight * distanceLight) +
        materialSpecularColor * lightColor * lightIntensity * pow(cosAlpha,5) / (distanceLight * distanceLight), 1);
}