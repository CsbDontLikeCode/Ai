#version 330 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomBlur;

void main()
{   
    // Normal
    // vec3 col = texture(screenTexture, TexCoords).rgb;
    // 
    // float gamma = 2.2;
    // col = pow(col, vec3(1.0/gamma));
    // 
    // FragColor = vec4(col, 1.0);

    //// Inversion
    // FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);

    //// Grey
    //FragColor = texture(screenTexture, TexCoords);
    //float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0;
    //FragColor = vec4(average, average, average, 1.0);

    // HDR 
    // const float gamma = 2.2;
    // vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
    // vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // mapped = pow(mapped, vec3(1.0 / gamma));
    // FragColor = vec4(mapped, 1.0);

    const float gamma = 2.2;
    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;  
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    hdrColor += bloomColor; // additive blending

    float exposure = 1.5;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    mapped = pow(mapped, vec3(1.0 / gamma));
    FragColor = vec4(mapped, 1.0);
} 