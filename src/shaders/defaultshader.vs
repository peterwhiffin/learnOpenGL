#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

uniform float offset;
uniform mat4 transform;

out vec3 bigColor;
out vec3 vertPos;
out vec2 texCoord;

void main()
{ 
    gl_Position = transform * vec4(aPos.x * -1, aPos.y * -1, aPos.z * -1, 1.0);
    bigColor = aColor;
    vertPos = aPos;
    texCoord = aTexCoord;
}  