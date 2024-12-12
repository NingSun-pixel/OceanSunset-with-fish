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

struct FishInstance {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 scale;
    float rotation;
};

class FishSimulation {
public:
    // 构造函数和析构函数
    FishSimulation(int numInstances, Camera& camera);
    ~FishSimulation();

    // 初始化鱼群实例
    void initFishInstances();
    // 加载鱼模型
    void loadFishModels(const std::vector<std::string>& fbxFiles);
    // 更新鱼群
    void updateFish(float deltaTime);
    // 渲染鱼群
    void renderFish(GLuint shaderProgram);

private:
    int numInstances;                             // 鱼群数量
    std::vector<FishInstance> instances;          // 鱼实例数据
    GLuint instanceVBO;                           // 实例化缓冲对象
    std::vector<Model> fishModels;                // 鱼模型列表
    Camera& camera;                               // 引用相机
};

#endif // FISH_SIMULATION_H
