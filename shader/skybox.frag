#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    // Sample from the cubemap texture
    vec4 color = texture(skybox, TexCoords);
    // Apply gamma correction
    color.rgb = pow(color.rgb, vec3(1.0/2.2));
    FragColor = color;
} 