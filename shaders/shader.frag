#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec3 fragPos;
in vec4 fragPosLightSpace;

out vec4 fColor;

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 pointLightPos;
uniform vec3 pointLightColor;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

// Fog și efecte atmosferice
uniform vec3 fogColor;
uniform bool fogEnabled;
uniform float time;
uniform bool rainEnabled;

// Render mode
uniform int renderMode;

// Parametri ajustabili pentru calitate vizuală îmbunătățită
uniform float ambientStrength = 0.2f;
uniform float diffuseStrength = 0.8f;
uniform float specularStrength = 0.6f;
uniform float shininess = 64.0f;

vec3 ambient;
vec3 diffuse;
vec3 specular;

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(normalize(fNormal), normalize(lightDir))), 0.005);

    // PCF îmbunătățit pentru umbre mai moi
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -3; x <= 3; ++x) {
        for(int y = -3; y <= 3; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 49.0; // 7x7 kernel

    if(projCoords.z > 1.0)
    shadow = 0.0;

    return shadow;
}

vec3 calculateWetReflection(vec3 baseColor, vec3 normal, vec3 viewDir) {
    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 5.0);
    vec3 reflection = reflect(-viewDir, normal);

    // Adăugăm variație temporală pentru efect de apă în mișcare
    float waterMovement = sin(time * 2.0 + fragPos.x * 0.5 + fragPos.z * 0.5) * 0.1;

    return mix(baseColor, vec3(1.0), (fresnel + waterMovement) * 0.5);
}

void computeDirectionalLight() {
    vec3 cameraPosEye = vec3(0.0f);
    vec3 lightDirN = normalize(lightDir);
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
    vec3 halfwayDir = normalize(lightDirN + viewDirN);

    // Ambient cu ocluzie bazată pe normală
    float occlusionFactor = max(dot(normalize(fNormal), vec3(0.0, 1.0, 0.0)), 0.0);
    ambient = ambientStrength * lightColor * (0.8 + 0.2 * occlusionFactor);

    // Diffuse cu wrapped lighting pentru tranziții mai moi
    float NdotL = max(dot(normalize(fNormal), lightDirN), 0.0);
    float wrappedDiffuse = (NdotL + 0.3) / 1.3;
    diffuse = diffuseStrength * wrappedDiffuse * lightColor;

    // Specular Blinn-Phong îmbunătățit
    float spec = pow(max(dot(normalize(fNormal), halfwayDir), 0.0), shininess);
    specular = specularStrength * spec * lightColor;
}

void computePointLight() {
    vec3 cameraPosEye = vec3(0.0f);
    vec3 lightDirN = normalize(pointLightPos - fPosEye.xyz);
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
    vec3 halfwayDir = normalize(lightDirN + viewDirN);

    float distance = length(pointLightPos - fPosEye.xyz);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

    float NdotL = max(dot(normalize(fNormal), lightDirN), 0.0);
    float wrappedDiffuse = (NdotL + 0.3) / 1.3;

    ambient += attenuation * ambientStrength * pointLightColor;
    diffuse += attenuation * diffuseStrength * wrappedDiffuse * pointLightColor;

    float spec = pow(max(dot(normalize(fNormal), halfwayDir), 0.0), shininess);
    specular += attenuation * specularStrength * spec * pointLightColor;
}

void main() {
    // Gestionare moduri de render
    if(renderMode == 1) { // Wireframe
        fColor = vec4(0.7, 0.7, 0.7, 1.0);
        return;
    }

    // Calculăm iluminarea de bază
    computeDirectionalLight();
    computePointLight();

    // Obținem culorile din texturi
    vec3 baseColor = texture(diffuseTexture, fTexCoords).rgb;
    vec3 specColor = texture(specularTexture, fTexCoords).rgb;

    // Calculăm umbra
    float shadow = ShadowCalculation(fragPosLightSpace);

    // Aplicăm iluminarea cu umbre
    vec3 lighting = (ambient + (1.0 - shadow * 0.85) * diffuse) * baseColor +
    (1.0 - shadow * 0.7) * specular * specColor;

    // Render modes specific
    if(renderMode == 2) { // Polygonal
        vec3 flatNormal = normalize(cross(dFdx(fragPos), dFdy(fragPos)));
        float flatDiffuse = max(dot(flatNormal, normalize(lightDir)), 0.0);
        lighting = vec3(0.7) * flatDiffuse;
    }
    else if(renderMode == 3) { // Smooth
        vec3 normal = normalize(fNormal);
        float smoothFactor = (normal.x + normal.y + normal.z) / 3.0;
        smoothFactor = smoothFactor * 0.5 + 0.5;
        lighting = vec3(smoothFactor);
    }
    else { // Solid (normal) cu efecte avansate
        // HDR tone mapping îmbunătățit
        lighting = lighting / (lighting + vec3(1.0));

        // Contrast și saturație îmbunătățite
        lighting = pow(lighting, vec3(1.1)); // Contrast
        float luminance = dot(lighting, vec3(0.299, 0.587, 0.114));
        lighting = mix(vec3(luminance), lighting, 1.2); // Saturație

        // Corecție gamma
        lighting = pow(lighting, vec3(1.0/2.2));

        // Efecte de ploaie
        if(rainEnabled) {
            vec3 viewDir = normalize(-fPosEye.xyz);
            lighting = calculateWetReflection(lighting, normalize(fNormal), viewDir);
            lighting *= 0.9; // Darkening pentru suprafețe ude
            lighting += specular * 0.4; // Highlights mai puternice pe suprafețe ude
        }

        // Fog volumetric îmbunătățit
        if(fogEnabled) {
            float dist = length(fPosEye.xyz);
            float fogAmount = 1.0 - exp(-dist * 0.008);

            // Variație de culoare bazată pe înălțime
            vec3 customFogColor = mix(
            fogColor,
            fogColor * 1.2,
            clamp(fragPos.y * 0.1, 0.0, 1.0)
            );

            // Variație temporală pentru efect dinamic
            float fogVariation = sin(time * 0.3 + fragPos.x * 0.05 + fragPos.z * 0.05) * 0.1;
            fogAmount = clamp(fogAmount + fogVariation, 0.0, 1.0);

            lighting = mix(lighting, customFogColor, fogAmount);
        }
    }

    fColor = vec4(lighting, 1.0);
}