#version 410 core

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 baseColor;

in vec3 fNormal;
in vec4 fPosEye;

out vec4 fColor;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

void computeLight()
{
    //compute eye space coordinates
    vec3 normalEye = normalize(fNormal);

    //normalize light direction
    vec3 lightDirN = normalize(lightDir);

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
}


void main()
{
    computeLight();
    //compute final vertex color
    vec3 color = min((ambient + diffuse) * baseColor + specular, 1.0f);
    fColor = vec4(color, 1.0f);
}