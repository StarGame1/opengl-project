#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 fNormal;
out vec4 fPosEye;
out vec2 fTexCoords;
out float visibility;
out vec4 fragPosLightSpace;
out vec3 fragPos;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceTrMatrix;
uniform float time;
uniform float windStrength;
uniform vec3 windDirection;

const float fogDensity = 0.003;
const float fogGradient = 1.5;

void main() {
    // Apply wind effect
    vec3 finalPosition = vPosition;
    if (vPosition.y > 1.0) { // Apply only to higher parts of objects
        float windFactor = (vPosition.y - 1.0) * 0.1;
        float windEffect = sin(time * 2.0 + vPosition.x * 0.5) * cos(time * 1.5 + vPosition.z * 0.3);
        finalPosition.x += windDirection.x * windStrength * windEffect * windFactor;
        finalPosition.z += windDirection.z * windStrength * windEffect * windFactor;
    }

    // Transform to world space
    fragPos = vec3(model * vec4(finalPosition, 1.0));

    // Transform to eye space
    fPosEye = view * model * vec4(finalPosition, 1.0);

    // Normal in eye space
    fNormal = normalMatrix * vNormal;

    // Pass texture coordinates
    fTexCoords = vTexCoords;

    // Calculate position in light space for shadows
    fragPosLightSpace = lightSpaceTrMatrix * vec4(fragPos, 1.0);

    // Calculate fog
    float distance = length(fPosEye.xyz);
    visibility = exp(-pow((distance * fogDensity), fogGradient));
    visibility = clamp(visibility, 0.0, 1.0);

    gl_Position = projection * view * model * vec4(finalPosition, 1.0);
}