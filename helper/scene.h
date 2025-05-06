#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "camera.h"

class Scene
{
protected:
	glm::mat4 model, view, projection;

public:
    int width;
    int height;

	Scene() : m_animate(true), width(800), height(600),
		model(1.0f), view(1.0f), projection(1.0f) { }
	virtual ~Scene() {}

	void setDimensions( int w, int h ) {
	    width = w;
	    height = h;
	}
	
    /**
      Load textures, initialize shaders, etc.
      */
    virtual void initScene(GLFWwindow *window) = 0;

    /**
      This is called prior to every frame.  Use this
      to update your animation.
      */
    virtual void update( float t ) = 0;

    /**
      Draw your scene.
      */
    virtual void render(GLFWwindow* window) = 0;

    /**
      Called when screen is resized
      */
    virtual void resize(int, int) = 0;
    
    void animate( bool value ) { m_animate = value; }
    bool animating() { return m_animate; }
    
protected:
	bool m_animate;
};
