#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    //vec3 position;
    vec3 direction;
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
in vec3 lightDir;

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
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));  
      
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightDir);   
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord));

    vec3 viewDir = normalize(-fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);      
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));  


    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}