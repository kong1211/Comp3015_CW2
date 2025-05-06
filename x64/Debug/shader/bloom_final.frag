#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sceneTexture;
uniform sampler2D bloomBlurTexture;
uniform float exposure;
uniform bool enableBloom;

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = texture(sceneTexture, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlurTexture, TexCoords).rgb;

    if(enableBloom)
      hdrColor += bloomColor; // Additive blending

    // Tone mapping (简单的 Reinhard) 和 Gamma 校正
    // vec3 result = vec3(1.0) - exp(-hdrColor * exposure); // Reinhard
    vec3 result = hdrColor / (hdrColor + vec3(1.0)); // Reinhard 简化
    result = pow(result, vec3(1.0/gamma)); // Gamma Correction

    FragColor = vec4(result, 1.0);
} 