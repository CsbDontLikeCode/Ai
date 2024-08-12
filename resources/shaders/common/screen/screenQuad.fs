#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{   
    // Normal
    vec3 col = texture(screenTexture, TexCoords).rgb;

    float gamma = 1.5;
    col = pow(col, vec3(1.0/gamma));

    FragColor = vec4(col, 1.0);

    //// Inversion
    // FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);

    //// Grey
    //FragColor = texture(screenTexture, TexCoords);
    //float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0;
    //FragColor = vec4(average, average, average, 1.0);
} 