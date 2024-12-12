#ifndef FISH_SIMULATION_H
#define FISH_SIMULATION_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include "Camera.h"
#include "ModelLoader.h"
#include "LightingManager.h"

struct FishInstance {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 scale;
    float rotation;
};

class FishSimulation {
public:
    FishSimulation(int numInstances, Camera& camera);
    ~FishSimulation();

    void initFishInstances();
    void loadFishModel(const std::string& modelPath);
    void updateFish(float deltaTime);
    void renderFish(GLuint shaderProgram);

private:
    int numInstances;
    std::vector<FishInstance> instances;
    GLuint instanceVBO;
    Model fishModel;
    Camera& camera;
};

#endif // FISH_SIMULATION_H