#include <glad/glad.h>
#include "scene.h"
#include <GLFW/glfw3.h>
#include "glutils.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#include <map>
#include <string>
#include <fstream>
#include <iostream>

class SceneRunner {
public:
    GLFWwindow * window;
    int fbw, fbh;
	bool debug;           // Set true to enable debug messages
    static bool imguiInitialized;  // Add static flag
    static bool imguiBackendInitialized;  // Add backend initialization flag

public:
    SceneRunner(const std::string & windowTitle, int width = WIN_WIDTH, int height = WIN_HEIGHT, int samples = 0) : debug(true) {
        // Initialize GLFW
        if( !glfwInit() ) exit( EXIT_FAILURE );

#ifdef __APPLE__
        // Select OpenGL 4.1
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
#else
        // Select OpenGL 4.6
        glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
        glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
#endif
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        if(debug) 
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        if(samples > 0) {
            glfwWindowHint(GLFW_SAMPLES, samples);
        }

        // Open the window
        window = glfwCreateWindow( WIN_WIDTH, WIN_HEIGHT, windowTitle.c_str(), NULL, NULL );
        if( ! window ) {
			std::cerr << "Unable to create OpenGL context." << std::endl;
            glfwTerminate();
            exit( EXIT_FAILURE );
        }
        glfwMakeContextCurrent(window);

        // Get framebuffer size
        glfwGetFramebufferSize(window, &fbw, &fbh);

        // tell GLFW to capture our mouse
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // Load the OpenGL functions.
        if(!gladLoadGL()) { exit(-1); }

        GLUtils::dumpGLInfo();

        // Initialization
        glClearColor(0.1f,0.1f,0.1f,1.0f);
#ifndef __APPLE__
		if (debug) {
			glDebugMessageCallback(GLUtils::debugCallback, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
			glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
				GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Start debugging");
		}
#endif
    }

    int run(Scene & scene) {
        scene.setDimensions(fbw, fbh);
        scene.initScene(window);
        scene.resize(fbw, fbh);

        // Initialize ImGui (if not already initialized)
        if (!imguiInitialized) {
            try {
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO(); (void)io;
                ImGui::StyleColorsDark();
                imguiInitialized = true;
            } catch (const std::exception& e) {
                std::cerr << "ImGui initialization failed: " << e.what() << std::endl;
            }
        }

        // Initialize ImGui backend (if not already initialized)
        if (imguiInitialized && !imguiBackendInitialized) {
            try {
                ImGui_ImplGlfw_InitForOpenGL(window, true);
                ImGui_ImplOpenGL3_Init("#version 330 core");
                imguiBackendInitialized = true;
            } catch (const std::exception& e) {
                std::cerr << "ImGui backend initialization failed: " << e.what() << std::endl;
            }
        }

        // Enter the main loop
        mainLoop(window, scene);

#ifndef __APPLE__
		if( debug )
			glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 1,
				GL_DEBUG_SEVERITY_NOTIFICATION, -1, "End debug");
#endif

        // ImGui cleanup
        if (imguiBackendInitialized) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            imguiBackendInitialized = false;
        }
        if (imguiInitialized) {
            ImGui::DestroyContext();
            imguiInitialized = false;
        }

        // Close window and terminate GLFW
        glfwTerminate();

        // Exit program
        return EXIT_SUCCESS;
    }

    static std::string parseCLArgs(int argc, char ** argv, std::map<std::string, std::string> & sceneData) {
        if( argc < 2 ) {
            printHelpInfo(argv[0], sceneData);
            exit(EXIT_FAILURE);
        }

        std::string recipeName = argv[1];
        auto it = sceneData.find(recipeName);
        if( it == sceneData.end() ) {
            printf("Unknown recipe: %s\n\n", recipeName.c_str());
            printHelpInfo(argv[0], sceneData);
            exit(EXIT_FAILURE);
        }

        return recipeName;
    }

private:
    static void printHelpInfo(const char * exeFile,  std::map<std::string, std::string> & sceneData) {
        printf("Usage: %s recipe-name\n\n", exeFile);
        printf("Recipe names: \n");
        for( auto it : sceneData ) {
            printf("  %11s : %s\n", it.first.c_str(), it.second.c_str());
        }
    }

    void mainLoop(GLFWwindow * window, Scene & scene) {
        while( ! glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE) ) {
            GLUtils::checkForOpenGLError(__FILE__,__LINE__);
			
            scene.update(float(glfwGetTime()));
            scene.render(window);

            glfwSwapBuffers(window);

            glfwPollEvents();
			int state = glfwGetKey(window, GLFW_KEY_SPACE);
			if (state == GLFW_PRESS)
				scene.animate(!scene.animating());
        }
    }
};
