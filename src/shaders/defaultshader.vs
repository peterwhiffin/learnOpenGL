#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aOffset;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normalMat;
uniform vec3 LightPos;
uniform vec3 LightDir;
uniform float time;

out vec3 bigColor;
out vec3 vertPos;
out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;
out vec3 lightPos;
out vec3 lightDir;

void main()
{ 
    gl_Position = projection * view * model * vec4(aPos + aOffset, 1.0);
    vertPos = aPos;
    texCoord = aTexCoord;
    normal = mat3(normalMat) * aNormal;
    //normal = aNormal;
    fragPos = vec3(view * model * vec4(aPos, 1.0));
    lightDir = vec3(view * vec4(LightDir, 0.0));
    //lightDir = LightDir;
}  