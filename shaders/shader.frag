#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec3 fragPos;
in vec4 fragPosLightSpace;

out vec4 fColor;

// Existing uniforms
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 pointLightPos;
uniform vec3 pointLightColor;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;
uniform vec3 fogColor;
uniform bool fogEnabled;
uniform float time;
uniform bool rainEnabled;
uniform int renderMode;

// New PBR parameters
uniform float metallic = 0.0;
uniform float roughness = 0.5;
uniform float ao = 1.0;

// Enhanced visual parameters
uniform float exposure = 1.0;
uniform float saturation = 1.2;
uniform float contrast = 1.1;

const float PI = 3.14159265359;

// PBR functions
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Enhanced shadow calculation with PCSS (Percentage Closer Soft Shadows)
float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(normalize(fNormal), normalize(lightDir))), 0.005);

    // PCSS
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    int pcfRadius = 3;

    for(int x = -pcfRadius; x <= pcfRadius; ++x) {
        for(int y = -pcfRadius; y <= pcfRadius; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= pow(2.0 * pcfRadius + 1.0, 2);

    return shadow;
}

// Color grading
vec3 colorGrade(vec3 color) {
    // Exposure tone mapping
    color = vec3(1.0) - exp(-color * exposure);

    // Saturation adjustment
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luminance), color, saturation);

    // Contrast adjustment
    color = pow(color, vec3(contrast));

    return color;
}

void main() {
    // Skip PBR for special render modes
    if(renderMode != 0) {
        // Handle non-PBR render modes
        if(renderMode == 1) { // Wireframe
            fColor = vec4(0.7, 0.7, 0.7, 1.0);
            return;
        }
        else if(renderMode == 2) { // Polygonal
            vec3 flatNormal = normalize(cross(dFdx(fragPos), dFdy(fragPos)));
            float flatDiffuse = max(dot(flatNormal, normalize(lightDir)), 0.0);
            fColor = vec4(vec3(0.7) * flatDiffuse, 1.0);
            return;
        }
        else if(renderMode == 3) { // Smooth
            vec3 normal = normalize(fNormal);
            float smoothFactor = (normal.x + normal.y + normal.z) / 3.0;
            smoothFactor = smoothFactor * 0.5 + 0.5;
            fColor = vec4(vec3(smoothFactor), 1.0);
            return;
        }
        return;
    }

    vec3 N = normalize(fNormal);
    vec3 V = normalize(-fPosEye.xyz);
    vec3 L = normalize(lightDir);
    vec3 H = normalize(V + L);

    // Sample textures
    vec3 albedo = texture(diffuseTexture, fTexCoords).rgb;
    float specularStrength = texture(specularTexture, fTexCoords).r;

    // Calculate PBR parameters
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Reflectance equation
    vec3 Lo = vec3(0.0);

    // Calculate per-light radiance
    vec3 radiance = lightColor;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    // Ambient lighting
    vec3 ambient = vec3(0.03) * albedo * ao;

    // Calculate shadows
    float shadow = ShadowCalculation(fragPosLightSpace);
    vec3 color = ambient + (1.0 - shadow) * Lo;

    // Add rain effect
    if(rainEnabled) {
        float wetness = 0.7;
        color *= wetness;

        // Add water streaks
        float streak = sin(fragPos.y * 20.0 + time * 2.0) * 0.5 + 0.5;
        streak *= smoothstep(0.7, 0.9, sin(fragPos.x * 10.0 + time));
        color += streak * 0.1;
    }

    // Add fog
    if(fogEnabled) {
        float dist = length(fPosEye.xyz);
        float fogAmount = 1.0 - exp(-dist * 0.02);
        fogAmount = clamp(fogAmount, 0.0, 1.0);

        // Dynamic fog
        vec3 dynamicFogColor = fogColor + vec3(sin(time * 0.5) * 0.1);
        color = mix(color, dynamicFogColor, fogAmount);
    }

    // Apply color grading
    color = colorGrade(color);

    // Final output
    fColor = vec4(color, 1.0);
}