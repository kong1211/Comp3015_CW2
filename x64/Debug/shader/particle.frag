#version 430

in vec3 outPosition;
in vec3 outVelocity;
in float outLifetime;
in float outSize;
flat in vec3 particleColorOut;

out vec4 fragColor;

uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform vec3 cameraPosition;

void main() {
    // Calculate distance for circular point sprite
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    
    // Discard fragment if distance is greater than 0.5
    if (dist > 0.5) {
        discard;
    }
    
    // Calculate glow effect
    float glow = 1.0 - smoothstep(0.0, 0.5, dist);
    glow = pow(glow, 0.2);  // Restore to previous glow intensity
    
    // Calculate distance to camera
    float distanceToCamera = length(outPosition - cameraPosition);
    float distanceFactor = 1.0 - smoothstep(0.0, 100.0, distanceToCamera);
    
    // Output color with enhanced glow effect
    vec3 finalColor = particleColorOut * (1.0 + glow * 15.0);  // Reduce glow intensity
    fragColor = vec4(finalColor, glow * distanceFactor); // Restore transparency calculation
} 