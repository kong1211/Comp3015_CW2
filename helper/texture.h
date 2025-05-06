#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <string>

class Texture {
public:
    Texture();
    ~Texture();

    bool loadTexture(const std::string& filename, bool flip = true);
    void bind(GLenum textureUnit = GL_TEXTURE0);
    void unbind();
    GLuint getID() const { return textureID; }

private:
    GLuint textureID;
    int width;
    int height;
    int channels;
};

#endif // TEXTURE_H 