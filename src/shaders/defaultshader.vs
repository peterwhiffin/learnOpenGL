#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aOffset;

layout (std140) uniform Matrices{
        mat4 projection;
        mat4 view;
    };

uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;
uniform mat4 normalMat;
uniform float time = 0.0;
uniform mat4 lightSpaceMatrix;

out vec3 bigColor;
out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;
out vec3 originalFragPos;
out vec3 lightPos;
out vec4 FragPosLightSpace;


void main()
{    
    originalFragPos = aPos;
    fragPos = vec3(model * vec4(aPos, 1.0));
    normal = mat3(normalMat) * aNormal;  
    texCoord = aTexCoord;   
    FragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
    gl_Position = projection * view * vec4(fragPos, 1.0);
}  
