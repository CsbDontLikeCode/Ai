#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform sampler2D skybox;

void main()
{    
    
    FragColor = texture(skybox, TexCoords);
    // vec4 color = texture(skybox, TexCoords);
    float brightness = dot(FragColor.rgb, vec3(0.35, 0.35, 0.35));
    if(brightness > 1.0)
        BrightColor = FragColor;
}