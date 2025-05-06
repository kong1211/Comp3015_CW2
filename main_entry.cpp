#include <iostream>
#include "helper/scene.h"
#include "helper/scenerunner.h"
#include "scenebasic_uniform.h"

// Main program entry point
int main(int argc, char* argv[]) {
    try {
        // Create scene runner
        std::cout << "OPEN OPENGL..." << std::endl;
        SceneRunner runner("Shader_Basics");
        
        // Create scene
        std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new SceneBasic_Uniform());
        
        // Run scene
        return runner.run(*scene);
    }
    catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 