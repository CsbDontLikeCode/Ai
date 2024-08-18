#version 330 core

in vec3 FragPos;
in vec2 TexCoord;

out vec4 FragColor;

struct DirLight {
    vec3 direction;
    vec3 color;
};

uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;

uniform DirLight dirLight;
uniform vec3 viewPos;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main()
{
	vec3 normal = texture(NormalMap, TexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 fragColor = CalcDirLight(dirLight, normal, viewDir);

	FragColor = vec4(fragColor, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 ambient = vec3(0.1, 0.1, 0.1) * light.color * texture(DiffuseMap, TexCoord).rgb;
    vec3 diffuse = light.color * diff * texture(DiffuseMap, TexCoord).rgb;
    // vec3 diffuse = texture(DiffuseMap, TexCoord).rgb;
    vec3 specular = light.color * spec * texture(DiffuseMap, TexCoord).rgb;
    return (ambient + diffuse + specular);
}