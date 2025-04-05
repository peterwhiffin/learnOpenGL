#version 460 core

#define NR_POINT_LIGHTS 4

out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse0;
    sampler2D texture_specular0;
    sampler2D depthMap;
    sampler2D blueNoise;
    samplerCube skybox;
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

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 color, vec3 ambient);  
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);  
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;
in vec3 originalFragPos;
in vec4 FragPosLightSpace;
in mat4 u_lightMatrix;

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform vec3 baseColor;
uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;

uniform mat4 u_invViewProj;
uniform mat4 u_invLightViewProj;
//uniform mat4 u_lightMatrix;
uniform ivec2 u_screenSize = ivec2(800, 600);
uniform int NUM_STEPS = 32;
uniform float intensity = .035;
uniform float noiseOffset = 1.0;
uniform float u_beerPower = 1.0;
uniform float u_powderPower = 1.0;
uniform vec2 screenResolution;

vec4 diffuseTexColor;
vec4 specularTexColor;

float chebyshevUpperBound(vec2 moments, float currentDepth)
{
    float p = 0.0; // Default to full light if variance is 0
    float mean = moments.x;
    float variance = moments.y - (mean * mean);
    variance = max(variance, 0.00002); // Avoid negative variance

    float d = currentDepth - mean;
    float pMax = variance / (variance + d * d);
    
    if (currentDepth > mean)
        p = pMax;
    else
        p = 1.0;

    return p;
}


float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(material.depthMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    vec3 normal = normalize(normal);
    vec3 lightDir = normalize(dirLight.direction - fragPos);
    float diffuseFactor = dot(normal, -lightDir);
    // calculate bias (based on depth map resolution and slope)
    float bias = mix(0.005, 0.0, diffuseFactor);

    float shadow = 0.0;
    vec2 texelSize = vec2(1.0) / vec2(textureSize(material.depthMap, 0));

    int count = 0;
    int samples = 1;

    for(int x = -samples; x <= samples; ++x)
    {
        for(int y = -samples; y <= samples; ++y)
        {
            count++;
            float shad;
            float pcfDepth = texture(material.depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shad = currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
            shadow += shad;        
        }    
    }

    shadow /= count;

    if(projCoords.z > 1.0)
        shadow = 0.0;
    
    return shadow;
}

float remap(float value, float inputMin, float inputMax, float outputMin, float outputMax) {
  float normalizedValue = (value - inputMin) / (inputMax - inputMin);
  return normalizedValue * (outputMax - outputMin) + outputMin;
}

vec3 WorldPosFromDepth(float depth, vec2 screenSize, mat4 invViewProj)
{
  float z = depth * 2.0 - 1.0; // [0, 1] -> [-1, 1]
  vec2 normalized = gl_FragCoord.xy / screenSize; // [0.5, u_viewPortSize] -> [0, 1]
  vec4 clipSpacePosition = vec4(normalized * 2.0 - 1.0, z, 1.0); // [0, 1] -> [-1, 1]

  // undo view + projection
  vec4 worldSpacePosition = invViewProj * clipSpacePosition;
  worldSpacePosition /= worldSpacePosition.w;

  return worldSpacePosition.xyz;
}

vec3 LightWorldPosFromDepth(float depth, vec2 screenSize, mat4 invViewProj)
{
  float z = depth * 2.0 - 1.0; // [0, 1] -> [-1, 1]
  vec2 normalized = gl_FragCoord.xy / screenSize; // [0.5, u_viewPortSize] -> [0, 1]
  vec4 clipSpacePosition = vec4(normalized * 2.0 - 1.0, z, 1.0); // [0, 1] -> [-1, 1]

  // undo view + projection
  vec4 worldSpacePosition = invViewProj * clipSpacePosition;
  worldSpacePosition /= worldSpacePosition.w;

  return worldSpacePosition.xyz;
}

void main()
{  

    //  float shadow = texture(material.depthMap, texCoord).r; // Direct depth map sample
    // FragColor = vec4(vec3(gl_FragCoord.z), 1.0); // Visualize depth
    // return;
diffuseTexColor = texture(material.texture_diffuse0, texCoord);
specularTexColor = texture(material.texture_specular0, texCoord);
if(diffuseTexColor.a < 0.1f)
        discard;

    // properties
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);
    // phase 1: Directional lighting
     
    // phase 2: Point lights
 //   for(int i = 0; i < NR_POINT_LIGHTS; i++)
//        result += CalcPointLight(pointLights[i], norm, fragPos, viewDir);    
    // phase 3: Spot light
//    result += CalcSpotLight(spotLight, norm, fragPos, viewDir);    

    

    //result = mix(result, fogColor, remap(fragPos.z - viewPos.z, distance(viewPos, viewDir * fogStart), distance(viewPos, viewDir * fogEnd), 0.0f, 1.0f));

  const vec3 rayEnd = WorldPosFromDepth(gl_FragCoord.z, u_screenSize, u_invViewProj);
  const vec3 rayStart = WorldPosFromDepth(0, u_screenSize, u_invViewProj);
  const vec3 rayDir = normalize(rayEnd - rayStart);
  const float totalDistance = distance(rayStart, rayEnd);
  const float rayStep = totalDistance / NUM_STEPS;
  const vec2 noiseUV = gl_FragCoord.xy / textureSize(material.blueNoise, 0);
  vec3 rayPos = rayStart + rayDir * rayStep * texture(material.blueNoise, noiseUV).x * noiseOffset;
  
  float accum = 0.0;
  float ambientAccum = 0.0;
  vec3 ambient = dirLight.ambient;
//vec2 screenUV = gl_FragCoord.xy / screenResolution;
  for (int i = 0; i < NUM_STEPS; i++)
  {
    vec4 lightSpacePos = u_lightMatrix * vec4(rayPos, 1.0);
    float shadowCalc = ShadowCalculation(lightSpacePos);
    accum += 1.0 - shadowCalc + .03 * i * shadowCalc;
    rayPos += rayDir * rayStep;
  }

  const float d = accum * rayStep * intensity;
  const float powder = 1.0 - exp(-d * 2.0 * u_powderPower);
  const float beer = exp(-d * u_beerPower);


  vec3 result = CalcDirLight(dirLight, norm, viewDir, baseColor, ambient);
  result = pow(result, vec3(1.0/2.2));

  result += (1.0 - beer) * powder;

  

    float luminance = dot(result.rgb, vec3(0.2126729f,  0.7151522f, 0.0721750f));
    FragColor = vec4(result, luminance);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 color, vec3 ambient){
    vec3 lightDir = normalize(light.direction - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    ambient = ambient * vec3(diffuseTexColor);
    vec3 diffuse = light.diffuse * diff * vec3(diffuseTexColor);
    vec3 specular = light.specular * spec * vec3(specularTexColor.r);

    //vec2 screenUV = gl_FragCoord.xy / screenResolution;

    float shadow = ShadowCalculation(FragPosLightSpace);   
    vec3 lighting = (ambient + (1.0 - shadow) * (specular + diffuse));   

    return lighting;
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

    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse0, texCoord));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse0, texCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular0, texCoord));

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

    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse0, texCoord));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse0, texCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular0, texCoord));
    
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
