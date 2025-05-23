///////////////////////////////////////////
/* Some code studied from LearnOpenGL */
/* https://learnopengl.com */
///////////////////////////////////////////

#include "common.h"
#include "helper/glutils.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Add stb_image_resize header file
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

#include "scenebasic_uniform.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include "helper/glutils.h"
#include <glm/gtc/matrix_transform.hpp>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

// Global variables definition
float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
bool firstMouse = true;
Camera camera(glm::vec3(0.0f, 1.5f, 3.0f));
float deltaTime = 0.0f;
float lastFrame = 0.0f;

SceneBasic_Uniform::SceneBasic_Uniform() : angle(0.0f)
{
}

SceneBasic_Uniform::~SceneBasic_Uniform()
{
    // Clean up OpenGL resources
    if (skyboxVAO != 0) {
        glDeleteVertexArrays(1, &skyboxVAO);
    }
    if (skyboxVBO != 0) {
        glDeleteBuffers(1, &skyboxVBO);
    }
    if (skyboxTexture != 0) {
        glDeleteTextures(1, &skyboxTexture);
    }
    if (sphereVAO != 0) {
        glDeleteVertexArrays(1, &sphereVAO);
    }
}

void SceneBasic_Uniform::initScene(GLFWwindow *window)
{
    compile();
    
    // Set mouse callback
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Get window size
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    width = w;
    height = h;
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Enable face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Enable GL_PROGRAM_POINT_SIZE
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    // Load ball textures
    loadBallTextures();
    
    // Initialize particle system
    initParticleSystem();

    // Setup depth map FBO
    setupDepthMapFBO();
    
    // --- Setup Ground Plane ---
    float planeVertices[] = {
        // positions          // normals         // texcoords
        // Triangle 1 (CCW from above)
         25.0f, -3.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 0.0f,
        -25.0f, -3.0f, -25.0f,  0.0f, 1.0f, 0.0f,  0.0f, 25.0f,
        -25.0f, -3.0f,  25.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,

        // Triangle 2 (CCW from above)
         25.0f, -3.0f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 0.0f,
         25.0f, -3.0f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f,
        -25.0f, -3.0f, -25.0f,  0.0f, 1.0f, 0.0f,  0.0f, 25.0f
    };
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // Normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // Texture coordinate attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    // Load plane texture (replace with your desired texture)
    int width, height, nrChannels;
    unsigned char *data = stbi_load("media/textures/wood.png", &width, &height, &nrChannels, 0); // Example texture
    if (data) {
        glGenTextures(1, &planeTexture);
        glBindTexture(GL_TEXTURE_2D, planeTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
        std::cout << "Successfully loaded plane texture" << std::endl;
    } else {
        std::cerr << "Failed to load plane texture" << std::endl;
    }
    // --- End Setup Ground Plane ---

    // Initialize skybox vertex data
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // Create skybox VAO and VBO
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Check OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error in initScene: " << err << std::endl;
    }

    // Load skybox textures
    std::vector<std::string> faces{
        "media/textures/skybox/px.png",  // right face
        "media/textures/skybox/nx.png",  // left face
        "media/textures/skybox/py.png",  // top face
        "media/textures/skybox/ny.png",  // bottom face
        "media/textures/skybox/pz.png",  // front face
        "media/textures/skybox/nz.png"   // back face
    };
    skyboxTexture = loadCubemap(faces);

    // Generate some items
    items.push_back(ItemInfo(0.0f, 0.0f, 0.0f));
}

void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
        prog.link();
        prog.use();
        
        skyboxProg.compileShader("shader/skybox.vert");
        skyboxProg.compileShader("shader/skybox.frag");
        skyboxProg.link();
        
        // Compile particle shaders
        particleProg.compileShader("shader/particle.vert");
        particleProg.compileShader("shader/particle.frag");
        particleProg.link();
        
        // Check if particle shader compiled and linked successfully
        if (!particleProg.isLinked()) {
            std::cerr << "Particle shader linking failed" << std::endl;
        }
        
        particleProg.use();

        // Compile depth shader
        depthProg.compileShader("shader/depth_shader.vert");
        depthProg.compileShader("shader/depth_shader.frag");
        depthProg.link();

    }
    catch (GLSLProgramException &e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

GLuint SceneBasic_Uniform::loadCubemap(std::vector<std::string> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                        0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);

    return textureID;
}

void SceneBasic_Uniform::renderSphere()
{
    const unsigned int X_SEGMENTS = 32;
    const unsigned int Y_SEGMENTS = 32;
    
    if (sphereVAO == 0)
    {
        // Generate sphere vertex data
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>());
                float yPos = std::cos(ySegment * glm::pi<float>());
                float zPos = std::sin(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>());
                
                // Position
                vertices.push_back(xPos);
                vertices.push_back(yPos);
                vertices.push_back(zPos);
                
                // Normal
                vertices.push_back(xPos);
                vertices.push_back(yPos);
                vertices.push_back(zPos);
                
                // Texture coordinate
                vertices.push_back(xSegment);
                vertices.push_back(ySegment);
            }
        }
        
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x < X_SEGMENTS; ++x)
            {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x + 1);
                
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x + 1);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
            }
        }
        
        // Generate VAO and VBO
        GLuint VBO, EBO;
        glGenVertexArrays(1, &sphereVAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        
        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        
        // Normal attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        
        // Texture coordinate attribute
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    }
    
    // Render sphere
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, X_SEGMENTS * Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void SceneBasic_Uniform::renderItems(GLSLProgram& shader, bool applyColor)
{
    // Render the collectible sphere
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, collectibleSpherePos);  // Use variable position
    model = glm::rotate(model, glm::radians(ballRotation), glm::vec3(0.0f, 1.0f, 0.0f));
    shader.setUniform("model", model);

    if (applyColor) {
        shader.setUniform("overrideColor", glm::vec3(-1.0f, -1.0f, -1.0f)); // Disable color override, use texture color
        glActiveTexture(GL_TEXTURE0); // Texture unit 0 for main texture
        glBindTexture(GL_TEXTURE_2D, ballTextureBlue);
        shader.setUniform("ballTexture", 0);
    }

    renderSphere();

    if (applyColor) {
        // Restore default color if needed (though overrideColor >= 0 check handles this)
        shader.setUniform("overrideColor", glm::vec3(-1.0f, -1.0f, -1.0f));
    }

    // Render other items if any
}

void SceneBasic_Uniform::renderSkybox(const glm::mat4& view, const glm::mat4& projection)
{
    if (skyboxTexture == 0) {
        std::cerr << "Error: Skybox texture is not loaded!" << std::endl;
        return;
    }

    // Disable depth writing
    glDepthMask(GL_FALSE);
    // Use GL_LEQUAL for depth testing
    glDepthFunc(GL_LEQUAL);
    
    // Bind skybox texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    
    // Use skybox shader program
    skyboxProg.use();
    
    // Set view and projection matrices
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view)); // Remove translation part
    skyboxProg.setUniform("view", skyboxView);
    skyboxProg.setUniform("projection", projection);
    skyboxProg.setUniform("skybox", 0);
    
    // Render skybox
    glBindVertexArray(skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    // Restore default depth testing
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
}

void SceneBasic_Uniform::processInput(GLFWwindow *window)
{
    // Calculate frame time
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Process keyboard input
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // WASD movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void SceneBasic_Uniform::render(GLFWwindow* window)
{
    processInput(window);

    // 1. Depth Map Pass: Render scene from light's perspective
    // --------------------------------------------------------
    glm::vec3 lightPosition = glm::vec3(10.0f, 15.0f, 10.0f); // Light position used for shadows
    float near_plane = 1.0f, far_plane = 30.5f;
    glm::mat4 lightProjection = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(lightPosition, 
                                      glm::vec3(0.0f, 0.0f, 0.0f), // Look at origin
                                      glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;

    depthProg.use();
    depthProg.setUniform("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        // Render objects that cast shadows (e.g., the blue sphere)
        renderSceneForShadow(depthProg);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Reset viewport to screen size
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. Final Rendering Pass: Render scene from camera's perspective
    // --------------------------------------------------------------
    prog.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    prog.setUniform("projection", projection);
    prog.setUniform("view", view);
    prog.setUniform("viewPos", camera.Position);
    prog.setUniform("lightPos", lightPosition); // Use the same light position
    prog.setUniform("lightSpaceMatrix", lightSpaceMatrix);

    // Bind depth map texture
    glActiveTexture(GL_TEXTURE1); // Use a different texture unit
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    prog.setUniform("shadowMap", 1); // Pass texture unit index

    // Render the scene normally
    // --- Render Plane --- 
    prog.setUniform("overrideColor", glm::vec3(-1.0f)); // Ensure no override color for plane
    prog.setUniform("material.specular", glm::vec3(0.1f)); // Less specular for wood
    prog.setUniform("material.shininess", 16.0f);
    prog.setUniform("reflectivity", 0.1f); // Less reflective
    glm::mat4 planeModel = glm::mat4(1.0f);
    prog.setUniform("model", planeModel);
    prog.setUniform("ballTexture", 0); 
    renderPlane();
    // --- End Render Plane ---

    // --- Render Items (Sphere) --- 
    // Reset material properties for the sphere if needed
    prog.setUniform("material.specular", glm::vec3(0.5f, 0.5f, 0.5f)); 
    prog.setUniform("material.shininess", 32.0f);
    prog.setUniform("reflectivity", 0.5f); 
    renderItems(prog, true); // Use the modified renderItems
    // --- End Render Items ---
    
    renderSkybox(view, projection);
    renderParticles();

    // Unbind shadow map texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0); // Reset active texture unit

    // Add state restoration
    glBindVertexArray(0);         // Unbind VAO
    glUseProgram(0);              // Unbind Program
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // Unbind Cube Map texture

    // Render the UI
    renderUI();
}

void SceneBasic_Uniform::update(float t)
{
    // Update particle system
    updateParticles(t);
    
    // Update ball rotation angle
    ballRotation += t * 50.0f;
    if (ballRotation > 360.0f)
        ballRotation -= 360.0f;

    // --- Collectible Sphere Logic ---
    float distance = glm::distance(camera.Position, collectibleSpherePos);
    if (distance < collectionDistance)
    {
        score++;
        // Respawn the sphere at a random position on the XZ plane within radius 10
        float angle = glm::radians((float)(rand() % 360));
        float radius = (float)(rand() % 100) / 10.0f; // 0.0 to 9.9
        collectibleSpherePos.x = radius * cos(angle);
        collectibleSpherePos.y = 0.0f; // Keep it on the ground level (adjust if needed)
        collectibleSpherePos.z = radius * sin(angle);
        
        // Ensure it doesn't spawn too close to the player immediately
        while (glm::distance(camera.Position, collectibleSpherePos) < collectionDistance * 2.0f) {
            angle = glm::radians((float)(rand() % 360));
            radius = (float)(rand() % 100) / 10.0f;
            collectibleSpherePos.x = radius * cos(angle);
            collectibleSpherePos.z = radius * sin(angle);
        }
        
        std::cout << "Collected! Score: " << score << std::endl; // Debug output
    }
    // --- End Collectible Sphere Logic ---
}

void SceneBasic_Uniform::resize(int w, int h)
{
    width = w;
    height = h;
    glViewport(0, 0, w, h);
}

void SceneBasic_Uniform::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void SceneBasic_Uniform::loadBallTextures()
{
    // Load candy ball texture (formerly blue plastic ball)
    int width, height, nrChannels;
    unsigned char *data = stbi_load("media/textures/Candy.png", &width, &height, &nrChannels, 0);
    if (data) {
        glGenTextures(1, &ballTextureBlue);
        glBindTexture(GL_TEXTURE_2D, ballTextureBlue);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
        std::cout << "Successfully loaded candy ball texture" << std::endl;
    } else {
        std::cerr << "Failed to load candy ball texture" << std::endl;
    }

    // Load yellow metal ball texture
    data = stbi_load("media/textures/ball_yellow.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glGenTextures(1, &ballTextureYellow);
        glBindTexture(GL_TEXTURE_2D, ballTextureYellow);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
        std::cout << "Successfully loaded yellow ball texture" << std::endl;
    } else {
        std::cerr << "Failed to load yellow ball texture" << std::endl;
    }

    // Load green ceramic ball texture
    data = stbi_load("media/textures/ball_green.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glGenTextures(1, &ballTextureGreen);
        glBindTexture(GL_TEXTURE_2D, ballTextureGreen);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
        std::cout << "Successfully loaded green ball texture" << std::endl;
    } else {
        std::cerr << "Failed to load green ball texture" << std::endl;
    }
}

// Add radians conversion function
float radians(float degrees) {
    return degrees * glm::pi<float>() / 180.0f;
}

void SceneBasic_Uniform::initParticleSystem()
{
    // Create particle VAO and VBO
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleBuffer);
    glGenBuffers(1, &transformFeedbackBuffer);

    // Bind VAO
    glBindVertexArray(particleVAO);

    // Initialize particle data
    std::vector<Particle> particles(particleCount);
    for (int i = 0; i < particleCount; i++) {
        // Create a ring in the XZ plane
        float theta = radians(360.0f * (float(i) / float(particleCount)));
        particles[i].position = emitterPosition + glm::vec3(
            emitterRadius * cos(theta),
            0.0f,  // Keep in XZ plane
            emitterRadius * sin(theta)
        );
        
        // Set initial velocity
        glm::vec3 direction = glm::normalize(particles[i].position - emitterPosition);
        particles[i].velocity = glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)) * particleSpeed;
        
        particles[i].life = 1.0f;
        particles[i].size = 5000.0f; // Greatly increase particle size
    }

    // Bind and fill particle buffer
    glBindBuffer(GL_ARRAY_BUFFER, particleBuffer);
    glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), &particles[0], GL_DYNAMIC_DRAW);

    // Set vertex attributes
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, position));
    // Velocity
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, velocity));
    // Lifetime
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, life));
    // Size
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, size));

    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneBasic_Uniform::updateParticles(float t)
{
    // Update particle positions and lifetimes
    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleBuffer);

    Particle* particles = (Particle*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    if (particles) {
        for (int i = 0; i < particleCount; i++) {
            // Calculate rotation angle
            float angle = rotationSpeed * deltaTime;
            
            // Create rotation matrix (only in XZ plane)
            glm::mat3 rotationMatrix = glm::mat3(
                cos(angle), 0.0f, sin(angle),
                0.0f, 1.0f, 0.0f,
                -sin(angle), 0.0f, cos(angle)
            );
            
            // Apply rotation
            particles[i].position = emitterPosition + rotationMatrix * (particles[i].position - emitterPosition);
            
            // Update velocity
            glm::vec3 direction = glm::normalize(particles[i].position - emitterPosition);
            particles[i].velocity = glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)) * particleSpeed;
            
            // Update lifetime
            particles[i].life -= lifeDecay * deltaTime;
            
            // If particle lifetime ends, reset it
            if (particles[i].life <= 0.0f) {
                // Create a ring in the XZ plane
                float theta = radians(360.0f * (float(i) / float(particleCount)));
                particles[i].position = emitterPosition + glm::vec3(
                    emitterRadius * cos(theta),
                    0.0f,  // Keep in XZ plane
                    emitterRadius * sin(theta)
                );
                
                // Set initial velocity
                glm::vec3 direction = glm::normalize(particles[i].position - emitterPosition);
                particles[i].velocity = glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)) * particleSpeed;
                
                particles[i].life = 1.0f;
                particles[i].size = 5000.0f; // Greatly increase particle size
            }
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SceneBasic_Uniform::renderParticles()
{
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Use additive blending to make particles brighter

    // Use particle shader program
    particleProg.use();

    // Set transformation matrices
    setMatrices(particleProg);

    // Set camera position and direction
    particleProg.setUniform("cameraPosition", camera.Position);
    particleProg.setUniform("cameraRight", glm::normalize(glm::cross(camera.Front, camera.Up)));
    particleProg.setUniform("cameraUp", camera.Up);

    // Set particle parameters
    particleProg.setUniform("deltaTime", deltaTime);
    particleProg.setUniform("emitterPosition", emitterPosition);
    particleProg.setUniform("emitterRadius", emitterRadius);
    particleProg.setUniform("particleSpeed", particleSpeed);
    particleProg.setUniform("gravity", gravity);
    particleProg.setUniform("lifeDecay", lifeDecay);
    particleProg.setUniform("particleColor", particleColor);
    particleProg.setUniform("rotationSpeed", rotationSpeed);
    particleProg.setUniform("particleCount", particleCount);  // Add particle count uniform

    // Bind and draw particles
    glBindVertexArray(particleVAO);
    glDrawArrays(GL_POINTS, 0, particleCount);
    glBindVertexArray(0);

    // Disable blending
    glDisable(GL_BLEND);
}

void SceneBasic_Uniform::setMatrices(GLSLProgram& prog)
{
    // Set model matrix
    glm::mat4 model = glm::mat4(1.0f);
    prog.setUniform("model", model);
    
    // Set view matrix
    glm::mat4 view = camera.GetViewMatrix();
    prog.setUniform("view", view);
    
    // Set projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    prog.setUniform("projection", projection);
    
    // Set camera position
    prog.setUniform("cameraPosition", camera.Position);
    
    // Set camera direction
    glm::vec3 cameraRight = glm::normalize(glm::cross(camera.Front, camera.Up));
    glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, camera.Front));
    prog.setUniform("cameraRight", cameraRight);
    prog.setUniform("cameraUp", cameraUp);
    
    // Set rotation speed
    prog.setUniform("rotationSpeed", rotationSpeed);
}

void SceneBasic_Uniform::setupDepthMapFBO()
{
    // Create depth texture
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Clamp to border to avoid sampling outside the map, causing artifacts
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Areas outside map are fully lit
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  

    // Create depth FBO
    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE); // No color buffer needed
    glReadBuffer(GL_NONE);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind back to default framebuffer
}

void SceneBasic_Uniform::renderSceneForShadow(GLSLProgram& shader)
{
    // Render the collectible sphere to cast shadow
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, collectibleSpherePos); // Use variable position
    model = glm::rotate(model, glm::radians(ballRotation), glm::vec3(0.0f, 1.0f, 0.0f));
    shader.setUniform("model", model);
    renderSphere();

    // Add other objects here if they should cast shadows
    // Example: render a plane
    // model = glm::mat4(1.0f);
    // model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // Position the plane
    // model = glm::scale(model, glm::vec3(20.0f)); // Scale the plane
    // shader.setUniform("model", model);
    // renderPlane(); // Assuming you have a renderPlane function
}

void SceneBasic_Uniform::renderPlane()
{
    // Bind plane texture (use texture unit 0 as ballTexture might be bound to it)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planeTexture);
    // prog.setUniform("ballTexture", 0); // Assuming the main shader uses unit 0 for the primary texture
    
    // Set model matrix for the plane (identity matrix means it's centered at origin)
    glm::mat4 model = glm::mat4(1.0f);
    // prog.setUniform("model", model); // Assuming model uniform is set by the caller or globally

    // Bind plane VAO and draw
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SceneBasic_Uniform::renderUI()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create a simple window
    ImGui::Begin("Game Info");                          
    ImGui::Text("Score: %d", score); // Display the score
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    // Rendering ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
} 