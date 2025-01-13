#version 410

in vec3 fTexCoords;
out vec4 FragColor;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, fTexCoords);
}