#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aOffset;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 normalMat;
uniform float time = 0.0;

out vec3 bigColor;
out vec2 texCoords;
out vec3 normal;
out vec3 fragPos;
out vec3 lightPos;
const vec2 data[4] = vec2[](
vec2(-1.0,  1.0), 
vec2(-1.0, -1.0),
vec2( 1.0,  1.0), 
vec2( 1.0, -1.0));

// out vec3 pos;
//   out vec3 fsun;
//   uniform mat4 P;
//   uniform mat4 V;
//   uniform float time = 0.0;

//   const vec2 data[4] = vec2[](
//     vec2(-1.0,  1.0), vec2(-1.0, -1.0),
//     vec2( 1.0,  1.0), vec2( 1.0, -1.0));

//   void main()
//   {
//     gl_Position = vec4(data[gl_VertexID], 0.0, 1.0);
//     pos = transpose(mat3(V)) * (inverse(P) * gl_Position).xyz;
//     fsun = vec3(0.0, sin(time * 0.01), cos(time * 0.01));
//   }

void main()
{    
    fragPos = vec3(model * vec4(aPos, 1.0));
    normal = mat3(normalMat) * aNormal;  
    texCoords = aTexCoord;   
    gl_Position = projection * view * vec4(fragPos, 1.0);
}  
