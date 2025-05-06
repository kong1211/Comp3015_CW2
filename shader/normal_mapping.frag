#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform vec3 spotlightPos;
uniform vec3 spotlightDir;
uniform float spotlightCutOff;
uniform float spotlightOuterCutOff;
uniform vec3 spotlightColor;
uniform float fogDensity;

void main()
{           
    // obtain normal from normal map in range [0,1]
    vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
   
    // get diffuse color
    vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.2) * spec;

    vec3 spotDir = normalize(spotlightPos - fs_in.FragPos);
    float theta = dot(spotDir, normalize(-spotlightDir));
    float epsilon = spotlightCutOff - spotlightOuterCutOff;
    float intensity = clamp((theta - spotlightOuterCutOff) / epsilon, 0.0, 1.0);
    if(intensity <= 0.01) intensity = 0.0;
    float spotlightDiff = max(dot(spotDir, normal), 0.0);
    vec3 spotlightDiffuse = spotlightDiff * spotlightColor;
    float spotlightSpec = pow(max(dot(normalize(spotDir + viewDir), normal), 0.0), 32.0);
    vec3 spotlightSpecular = vec3(0.5) * spotlightSpec * spotlightColor;

    vec3 result = ambient + diffuse + specular + (spotlightDiffuse + spotlightSpecular) * intensity;

    float distance = length(fs_in.FragPos - fs_in.TangentViewPos);
    float fogStart = 5.0;
    float fogEnd = 15.0;
    float fogFactor = (fogEnd - distance) / (fogEnd - fogStart);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    vec3 fogColor = vec3(0.7, 0.8, 1.0);
    result = mix(fogColor, result, fogFactor);
    FragColor = vec4(result, 1.0);
} 