#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 FragColor;


in vec3 bigColor;
in vec3 vertPos;
in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;
in vec3 lightPos;

uniform sampler2D mainTex;
uniform sampler2D texture2;
uniform Material material;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform Light light;

void main()
{
    //FragColor = mix(texture(mainTex, texCoord), texture(texture2, texCoord), 0.2);
    vec3 ambient = light.ambient * material.ambient;  
      
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);   
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    vec3 viewDir = normalize(-fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);      
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}