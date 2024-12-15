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
    glm::vec4 color; // 新增的颜色属性
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
    std::vector<glm::vec3> centers;                // 多中心点位置
    glm::vec3 leaderPosition;
};

#endif // FISH_SIMULATION_H