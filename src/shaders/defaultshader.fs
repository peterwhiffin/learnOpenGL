#version 330 core

#define NR_POINT_LIGHTS 4

out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight{
    vec3 position;
    vec3 direction;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 color);  
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);  
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;
in vec3 originalFragPos;

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform vec3 baseColor;
void main()
{  
    // properties
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);
    // phase 1: Directional lighting
     vec3 result = CalcDirLight(dirLight, norm, viewDir, baseColor);
    // phase 2: Point lights
 //   for(int i = 0; i < NR_POINT_LIGHTS; i++)
//        result += CalcPointLight(pointLights[i], norm, fragPos, viewDir);    
    // phase 3: Spot light
//    result += CalcSpotLight(spotLight, norm, fragPos, viewDir);    

    
    FragColor = vec4(result, 1.0f);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 color){
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));
  //  vec3 ambient = light.ambient * color;
 //   vec3 diffuse = light.diffuse * diff * color;
//    vec3 specular = light.specular * spec * color;

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    

    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, texCoord));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, texCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
} 

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    

    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, texCoord));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, texCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));
    
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    diffuse *= intensity;
    specular *= intensity;
    
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}
