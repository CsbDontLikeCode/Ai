#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  

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

    vec3 result = ambient * material.ambient + diffuse * material.diffuse + specular * material.specular;
    FragColor = vec4(result, 1.0);
} 