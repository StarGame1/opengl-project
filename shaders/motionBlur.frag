#version 410 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float blurStrength;  // va fi actualizat bazat pe viteza camerei
uniform float deltaTime;

void main() {
    vec2 texelSize = 1.0 / textureSize(screenTexture, 0);
    vec4 result = vec4(0.0);

    // Număr mai mic de sample-uri pentru un efect mai subtil
    float samples = 3.0;
    float totalWeight = 0.0;

    for(float i = 0; i < samples; i++) {
        float weight = 1.0 - (i / samples);
        vec2 offset = vec2(texelSize.x * i * blurStrength, 0.0);

        result += texture(screenTexture, TexCoords + offset) * weight;
        result += texture(screenTexture, TexCoords - offset) * weight;
        totalWeight += 2.0 * weight;
    }

    FragColor = result / totalWeight;
}