#version 330 core

out vec4 FragColor;


in vec3 bigColor;
in vec3 vertPos;
in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

uniform sampler2D mainTex;
uniform sampler2D texture2;
uniform vec3 lightPos;
uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
    //FragColor = mix(texture(mainTex, texCoord), texture(texture2, texCoord), 0.2);

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);  

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}