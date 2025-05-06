#include "texture.h"
#include "stb_image.h"
#include <iostream>
#include <fstream>

Texture::Texture() : textureID(0), width(0), height(0), channels(0) {}

Texture::~Texture() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
}

bool fileExists(const std::string& filename) {
    std::ifstream file(filename.c_str());
    return file.good();
}

bool Texture::loadTexture(const std::string& filename, bool flip) {
    try {
        if (!fileExists(filename)) {
            std::cerr << "Texture file does not exist: " << filename << std::endl;
            return false;
        }

        std::cout << "Loading texture: " << filename << " (flip=" << (flip ? "true" : "false") << ")" << std::endl;
        
        stbi_set_flip_vertically_on_load(flip);
        
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
        if (!data) {
            std::cerr << "Failed to load texture: " << filename << " - " << stbi_failure_reason() << std::endl;
            return false;
        }

        std::cout << "Texture size: " << width << "x" << height << ", channels: " << channels << std::endl;
        
        if (width <= 0 || height <= 0 || channels <= 0) {
            std::cerr << "Invalid texture size or channel count: " << width << "x" << height << "x" << channels << std::endl;
            stbi_image_free(data);
            return false;
        }

        if (textureID != 0) {
            std::cout << "Deleting old texture ID: " << textureID << std::endl;
            glDeleteTextures(1, &textureID);
            textureID = 0;
        }

        glGenTextures(1, &textureID);
        if (textureID == 0) {
            std::cerr << "Failed to generate texture ID" << std::endl;
            stbi_image_free(data);
            return false;
        }
        
        std::cout << "Generated new texture ID: " << textureID << std::endl;
        glBindTexture(GL_TEXTURE_2D, textureID);

        GLenum format = GL_RGB; // Default value
        if (channels == 1)
            format = GL_RED;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;
        else {
            std::cerr << "Unsupported channel count: " << channels << std::endl;
            stbi_image_free(data);
            return false;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        
        // Check OpenGL error
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "OpenGL error when creating texture: " << err << std::endl;
            stbi_image_free(data);
            return false;
        }
        
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Check OpenGL error again
        err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "OpenGL error when setting texture parameters: " << err << std::endl;
        }

        stbi_image_free(data);
        std::cout << "Texture loaded successfully: " << filename << ", ID: " << textureID << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception when loading texture: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception when loading texture" << std::endl;
        return false;
    }
}

void Texture::bind(GLenum textureUnit) {
    try {
        if (textureID == 0) {
            std::cerr << "Warning: Attempting to bind invalid texture (ID=0)" << std::endl;
            return;
        }
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // Check OpenGL error
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "OpenGL error when binding texture: " << err << " (ID=" << textureID << ")" << std::endl;
        }
    } catch (...) {
        std::cerr << "Exception when binding texture" << std::endl;
    }
}

void Texture::unbind() {
    try {
        glBindTexture(GL_TEXTURE_2D, 0);
    } catch (...) {
        std::cerr << "Exception when unbinding texture" << std::endl;
    }
} 