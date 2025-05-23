#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"
#include "helper/texture.h"
#include "helper/camera.h"
#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "helper/stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <GLFW/glfw3.h>

// Global variables
extern float lastX;
extern float lastY;
extern bool firstMouse;
extern Camera camera;
extern float deltaTime;
extern float lastFrame;

// Item information structure, including position and color
struct ItemInfo {
    float x, y, z;  // Position
    float r, g, b;  // Color
    
    ItemInfo(float x, float y, float z) : x(x), y(y), z(z), r(1.0f), g(0.0f), b(0.0f) {}
    ItemInfo(float x, float y, float z, float r, float g, float b) : x(x), y(y), z(z), r(r), g(g), b(b) {}
};

// Particle structure
struct Particle {
    glm::vec3 position;  // Position
    glm::vec3 velocity;  // Velocity
    float life;          // Lifetime
    float size;          // Size
};

class SceneBasic_Uniform : public Scene
{
private:
    GLuint vaoHandle = 0;
    GLSLProgram prog;
    GLSLProgram normalMappingProg;
    GLSLProgram screenProg;
    GLSLProgram skyboxProg;
    GLSLProgram edgeProg;
    GLSLProgram particleProg;
    GLSLProgram depthProg; // Depth map shader program
    float angle = 0.0f;
    float fogDensity = 0.05f;
    Texture diffuseTexture;
    Texture normalMap;
    GLuint skyboxTexture = 0;
    
    // PBR textures
    GLuint specularMapID = 0;
    GLuint roughnessMapID = 0;
    GLuint metallicMapID = 0;
    GLuint aoMapID = 0;

    // framebuffer
    GLuint framebuffer = 0;
    GLuint textureColorbuffer = 0;
    GLuint rbo = 0;
    GLuint quadVAO = 0;
    GLuint quadVBO = 0;

    // Shadow mapping related
    GLuint depthMapFBO = 0;
    GLuint depthMapTexture = 0;
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);

    // skybox
    GLuint skyboxVAO = 0;
    GLuint skyboxVBO = 0;

    // spotlight parameters
    glm::vec3 spotlightPos = glm::vec3(0.0f, 0.0f, 2.0f);
    glm::vec3 spotlightDir = glm::vec3(0.0f, 0.0f, -1.0f);
    float spotlightCutOff = glm::cos(glm::radians(12.5f));
    float spotlightOuterCutOff = glm::cos(glm::radians(17.5f));
    glm::vec3 spotlightColor = glm::vec3(1.0f, 0.0f, 0.0f);

    // matrices
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);

    int postProcessMode = 0;

    // Game related variables
    std::vector<ItemInfo> items; // Item coordinates and colors
    float timeLeft = 600.0f; // 60 seconds timer
    float ballRotation = 0.0f; // Ball rotation angle
    float ballRotateSpeed = 20.0f; // Ball rotation speed
    glm::vec3 collectibleSpherePos = glm::vec3(8.0f, 0.0f, 0.0f); // Position of the collectible sphere
    int score = 0; // Player's score
    float collectionDistance = 1.5f; // How close the player needs to be to collect

    // Ball textures
    GLuint ballTextureBlue = 0;    // Blue plastic ball texture
    GLuint ballTextureYellow = 0;  // Yellow metal ball texture
    GLuint ballTextureGreen = 0;   // Green ceramic ball texture
    
    // Particle system variables
    GLuint particleVAO = 0;           // Particle vertex array object
    GLuint particleBuffer = 0;        // Particle buffer
    GLuint transformFeedbackBuffer = 0; // Transform feedback buffer
    const int particleCount = 100;     // Reduced particle count
    glm::vec3 emitterPosition = glm::vec3(8.0f, 0.0f, 0.0f);  // Particle ring surrounds blue sphere
    float emitterRadius = 5.0f;  // Keep radius
    float particleSpeed = 3.0f;  // Keep slower speed
    float gravity = 0.0f;
    float lifeDecay = 0.001f;  // Restore slower decay
    glm::vec3 particleColor = glm::vec3(1.0f, 0.0f, 0.0f);  // Change back to red
    float rotationSpeed = 1.0f;  // Restore slower rotation

    // Vertex data
    std::vector<float> planeVertices;
    std::vector<float> quadVertices;
    std::vector<float> skyboxVertices;
    std::vector<float> cubeVertices;

    GLuint sphereVAO = 0;
    GLuint planeVAO = 0; // VAO for the ground plane
    GLuint planeVBO = 0; // VBO for the ground plane
    GLuint planeTexture = 0; // Texture for the ground plane

    void processInput(GLFWwindow *window);
    void compile();
    void setupFBO();
    void loadCubemap();
    void loadPBRTextures(); // Load PBR related textures
    void createDefaultTextures(); // Helper method to create default textures
    void spawnItems(int count);
    void renderItems(const glm::mat4& view, const glm::mat4& projection);
    void renderItems(GLSLProgram& shader, bool applyColor); // Modified renderItems
    void loadBallTextures();
    void initParticleSystem();
    void updateParticles(float t);
    void renderParticles();
    void setupDepthMapFBO(); // Function to setup depth map FBO
    void renderSceneForShadow(GLSLProgram& shader); // Function to render scene for shadow map
    void setMatrices(GLSLProgram& prog);  // Declaration for setMatrices function
    GLuint loadCubemap(std::vector<std::string> faces);
    void renderSkybox(const glm::mat4& view, const glm::mat4& projection);
    void setMatrices(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
    void renderSphere();
    void renderPlane(); // Function to render the ground plane
    void renderUI();    // Function to render ImGui UI

public:
    SceneBasic_Uniform();
    ~SceneBasic_Uniform();  // Declaration for destructor

    void initScene(GLFWwindow* window);
    void update(float t);
    void render(GLFWwindow* window);
    void resize(int, int);

    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
};

#endif // SCENEBASIC_UNIFORM_H 
