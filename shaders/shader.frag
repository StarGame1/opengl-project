#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;

out vec4 fColor;

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// Add point light
uniform vec3 pointLightPos;
uniform vec3 pointLightColor;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

void computeDirectionalLight() {
    vec3 cameraPosEye = vec3(0.0f);
    vec3 normalEye = normalize(fNormal);
    vec3 lightDirN = normalize(lightDir);
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;
}

void computePointLight() {
    vec3 cameraPosEye = vec3(0.0f);
    vec3 normalEye = normalize(fNormal);
    vec3 lightDirN = normalize(pointLightPos - fPosEye.xyz);
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

    // compute attenuation
    float distance = length(pointLightPos - fPosEye.xyz);
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));

    // compute point light contribution
    float diff = max(dot(normalEye, lightDirN), 0.0f);
    vec3 pointDiffuse = diff * pointLightColor;

    vec3 reflection = reflect(-lightDirN, normalEye);
    float spec = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
    vec3 pointSpecular = specularStrength * spec * pointLightColor;

    // apply attenuation
    pointDiffuse *= attenuation;
    pointSpecular *= attenuation;

    // add to existing light
    diffuse += pointDiffuse;
    specular += pointSpecular;
}

void main() {
    computeDirectionalLight();
    computePointLight();

    ambient *= texture(diffuseTexture, fTexCoords).rgb;
    diffuse *= texture(diffuseTexture, fTexCoords).rgb;
    specular *= texture(specularTexture, fTexCoords).rgb;

    vec3 color = min((ambient + diffuse) + specular, 1.0f);
    fColor = vec4(color, 1.0f);
}