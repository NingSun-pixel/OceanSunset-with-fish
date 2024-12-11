#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include <stb_image.h>
#include <iostream>

class TextureManager {
public:
    static GLuint getTexture(const std::string& texturePath);
    static void cleanup();
private:
    static std::unordered_map<std::string, GLuint> textureCache;
};

#endif // TEXTUREMANAGER_H
