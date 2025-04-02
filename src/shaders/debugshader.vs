#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normalMat;
uniform float time = 0.0;
uniform vec3 aBaseColor;

out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;
out vec3 baseColor;
void main()
{    
    fragPos = vec3(model * vec4(aPos, 1.0));
    normal = mat3(normalMat) * aNormal;  
    texCoord = aTexCoord;   
    baseColor = aBaseColor;
    gl_Position = projection * view * vec4(fragPos, 1.0);
}  
