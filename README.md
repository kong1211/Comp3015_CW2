# 3D OpenGL Interactive Environment

## User Interaction Instructions

To interact with this application:

1. **Starting the Application**:
   - Launch the executable file from the project directory
   - The application will open in fullscreen mode with mouse capture enabled

2. **Controls**:
   - **Mouse**: Look around the 3D environment
   - **W, A, S, D**: Move forward, left, backward, and right
   - **ESC**: Exit the application or release mouse capture

3. **Game Mechanics**:
   - Collect the blue spheres to increase your score
   - You have a limited time to collect as many spheres as possible
   - Watch for the particle effects that indicate collectible items

## Program Code Explanation

The program is built using C++ and OpenGL, with a focus on modern shader-based rendering techniques. The core architecture includes:

### Key Classes and Functions

- **SceneBasic_Uniform**: The main scene class that handles rendering, updates, and user interaction
- **Camera**: Manages the 3D camera movement and perspective
- **GLSLProgram**: Handles shader compilation, linking, and uniform variable management
- **Particle System**: Implements a GPU-based particle system using transform feedback

### Rendering Pipeline

The rendering pipeline follows these steps:

1. Shadow map generation for dynamic shadows
2. Scene rendering with standard lighting models
3. Post-processing effects application
4. UI rendering using ImGui

### Shader Implementation

Multiple shaders work together to create the final image:

- **basic_uniform**: The main shader for rendering 3D objects with lighting
- **normal_mapping**: Enhances surface detail using normal maps
- **particle**: Manages the life cycle and appearance of particles
- **skybox**: Creates the environment backdrop
- **edge** and **framebuffer**: Handle post-processing effects

## Special Features of the Shader Program

This shader implementation stands out due to several unique features:

1. **Combined Shadow Mapping and Normal Mapping**: The shaders efficiently combine shadow mapping with normal mapping to create realistic lighting that respects both macro and micro surface details.

2. **GPU-Accelerated Particle System**: The particle system uses transform feedback to update all particles on the GPU, avoiding CPU-GPU transfer bottlenecks common in similar applications.

3. **Flexible Post-Processing Pipeline**: The implementation allows for dynamic switching between multiple post-processing effects at runtime.

Compared to similar shader programs, this implementation focuses on performance optimization while maintaining visual quality, particularly in the particle system and lighting calculations.

## Sources and Uniqueness

The foundation of this project began with inspiration from LearnOpenGL tutorials (https://learnopengl.com), which provided the basic framework for modern OpenGL programming. However, the implementation has been significantly extended and customized:

- The particle system was enhanced with a unique emission pattern and lifecycle management
- The game mechanics and interaction were entirely original creations

What makes this work unique is the combination of these elements into a cohesive real-time interactive environment, with special attention paid to:

- The visual feedback loop between player actions and particle effects
- The optimization of shader code for performance on consumer hardware
- The integration of ImGui for runtime parameter adjustment

## Development Environment

This project was developed and tested on:

- **Visual Studio Version**: Visual Studio 2022 Community Edition (Version 17.7.5)
- **Operating System**: Windows 11 
- **GPU**: NVIDIA GeForce RTX 4060
- **Required Libraries**: 
  - GLFW 3.3.8
  - GLM 0.9.9.8
  - Dear ImGui 1.89.3

## Additional Information

### Performance Considerations

The application is optimized to run at 60+ FPS on mid-range hardware. The most performance-intensive aspects are:

- Shadow mapping at high resolutions
- Particle system with large particle counts
- Post-processing effects when multiple are enabled simultaneously

### Future Enhancements

Planned future enhancements include:

- Implementation of PBR workflow
- Volumetric lighting effects
- Physics-based interactions between player and environment

### Known Issues

- Shadows may exhibit aliasing at certain viewing angles
- Particle effects may have reduced performance on integrated GPUs

## Links

- **GitHub Repository**: https://github.com/kong1211/Comp3015_CW2
- **Demo Video**: https://youtu.be/PexTRFCGurg
