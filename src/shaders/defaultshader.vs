#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform float offset;

out vec3 bigColor;
out vec3 vertPos;

void main()
{ 
    gl_Position = vec4(aPos.x * -1, aPos.y * -1, aPos.z * -1, 1.0);
    bigColor = aColor;
    vertPos = aPos;
}  