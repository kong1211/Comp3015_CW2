#version 430

// Input variables
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inVelocity;
layout(location = 2) in float inLifetime;
layout(location = 3) in float inSize;

// Output variables
out vec3 outPosition;
out vec3 outVelocity;
out float outLifetime;
out float outSize;
flat out vec3 particleColorOut;

// Uniform variables
uniform float deltaTime;
uniform vec3 emitterPosition;
uniform float emitterRadius;
uniform float particleSpeed;
uniform float gravity;
uniform float lifeDecay;
uniform vec3 particleColor;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform float rotationSpeed;
uniform int particleCount;

void main() {
    // Update particle position
    outPosition = inPosition;
    
    // Calculate rotation angle
    float angle = rotationSpeed * deltaTime;
    
    // Create rotation matrix (only in XZ plane)
    mat3 rotationMatrix = mat3(
        cos(angle), 0.0, sin(angle),
        0.0, 1.0, 0.0,
        -sin(angle), 0.0, cos(angle)
    );
    
    // Apply rotation
    outPosition = emitterPosition + rotationMatrix * (outPosition - emitterPosition);
    
    // Update velocity
    vec3 direction = normalize(outPosition - emitterPosition);
    outVelocity = cross(direction, vec3(0.0, 1.0, 0.0)) * particleSpeed;
    
    // Update particle lifetime
    outLifetime = inLifetime - lifeDecay * deltaTime;
    
    // Pass other attributes
    outSize = 1.5;  // Adjust to a smaller but visible size
    particleColorOut = particleColor;
    
    // If particle lifetime ends, reinitialize
    if (outLifetime <= 0.0) {
        // Create a ring in the XZ plane
        float theta = radians(360.0 * (float(gl_VertexID) / float(particleCount)));
        outPosition = emitterPosition + vec3(
            emitterRadius * cos(theta),
            0.0,  // Keep in XZ plane
            emitterRadius * sin(theta)
        );
        
        // Set initial velocity
        vec3 direction = normalize(outPosition - emitterPosition);
        outVelocity = cross(direction, vec3(0.0, 1.0, 0.0)) * particleSpeed;
        
        outLifetime = 1.0;
    }
    
    // Calculate particle position in screen space
    vec4 pos = projection * view * vec4(outPosition, 1.0);
    gl_Position = pos;
    
    // Set point size, greatly increased
    gl_PointSize = outSize * 100.0 * (1.0 / pos.w); // Significantly reduce point size factor
} 