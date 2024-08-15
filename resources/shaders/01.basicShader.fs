#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos; 
in vec4 FragPosLightSpace;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
    float shininess;
}; 

uniform vec3 lightPos; 
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform Material material;

uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - 0.01 > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    // float shadow = currentDepth - 0.01 > closestDepth  ? 1.0 : 0.0;

    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    
    float spec = 0.0;
    // Blinn-Phong Shading
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    // Phong Shading
    // vec3 reflectDir = reflect(-lightDir, norm);  
    // spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = specularStrength * spec * lightColor; 

    // Calculate shadow.
    float shadow = ShadowCalculation(FragPosLightSpace);         

    // vec3 result = ambient * material.ambient + diffuse * material.diffuse + specular * material.specular;
    vec3 result = ambient * material.ambient + (1.0 - shadow) * diffuse * material.diffuse +  (1.0 - shadow) * specular * material.specular;
    FragColor = vec4(result, 1.0);
} 