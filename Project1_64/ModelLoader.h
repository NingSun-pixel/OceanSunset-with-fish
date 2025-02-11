#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <filesystem>
#include "TextureManager.h"
using namespace std;

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct Model {
    unsigned int VAO, VBO, EBO;
    unsigned int textureID;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    string meshName;
};

void loadSingleModel(const std::string& path, Model& model);
void loadSeparateModels(const std::vector<std::string>& fbxFiles, std::vector<Model>& models);
std::vector<std::string> getAllFBXFiles(const std::string& folderPath);

#endif // MODELLOADER_H
