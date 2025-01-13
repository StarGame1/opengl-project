#version 410

layout (location = 0) in vec3 vPosition;

out vec3 fTexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    fTexCoords = vPosition;
    vec4 pos = projection * view * vec4(vPosition, 1.0);
    gl_Position = pos.xyww;
}