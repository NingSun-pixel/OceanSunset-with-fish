#include "FishSimulation.h"
#include <random>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp> // For glm::lerp

// 辅助函数：生成随机数
float randomFloat(float min, float max) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

// 构造函数
FishSimulation::FishSimulation(int numInstances, Camera& camera)
    : numInstances(numInstances), camera(camera) {}

// 析构函数
FishSimulation::~FishSimulation() {
    glDeleteBuffers(1, &instanceVBO);
}

void FishSimulation::initFishInstances() {
    instances.resize(numInstances);

    // 初始化多中心点
    centers = {
        glm::vec3(-15.0f, 10.0f, -15.0f),
        glm::vec3(15.0f, 15.0f, 15.0f),
        glm::vec3(-20.0f, 20.0f, 20.0f)
    };

    int numCenters = centers.size();

    for (int i = 0; i < numInstances; ++i) {
        // 随机分配到某个中心点
        glm::vec3 center = centers[i % numCenters];

        // 增加更大的随机偏移量以减少过于密集的效果
        instances[i].position = glm::vec3(
            randomFloat(center.x - 10.0f, center.x + 10.0f), // 扩大范围到 [-10, 10]
            randomFloat(center.y - 5.0f, center.y + 5.0f),   // 高度范围 [-5, 5]
            randomFloat(center.z - 10.0f, center.z + 10.0f)  // 扩大范围到 [-10, 10]
        );

        // 增加随机速度
        instances[i].velocity = glm::vec3(
            randomFloat(-0.3f, 0.3f),  // 调低初始速度
            randomFloat(-0.3f, 0.3f),
            randomFloat(-0.3f, 0.3f)
        );

        // 保持适当的缩放范围
        instances[i].scale = glm::vec3(randomFloat(1.5f, 2.0f));
        instances[i].rotation = randomFloat(-30.0f, 30.0f);


        // 随机生成颜色 (RGBA)
        instances[i].color = glm::vec4(
            randomFloat(0.2f, 1.0f), // 红色分量
            randomFloat(0.2f, 1.0f), // 绿色分量
            randomFloat(0.2f, 1.0f), // 蓝色分量
            1.0f                     // Alpha 不透明
        );
    }

    // 创建实例化缓冲区
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(FishInstance), instances.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


// 加载鱼模型
void FishSimulation::loadFishModel(const std::string& modelPath) {
    loadSingleModel(modelPath, fishModel); // 使用模型加载器
    if (fishModel.vertices.empty()) {
        std::cerr << "Failed to load fish model!" << std::endl;
        return;
    }

    glGenVertexArrays(1, &fishModel.VAO);
    glGenBuffers(1, &fishModel.VBO);
    glGenBuffers(1, &fishModel.EBO);

    glBindVertexArray(fishModel.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, fishModel.VBO);
    glBufferData(GL_ARRAY_BUFFER, fishModel.vertices.size() * sizeof(Vertex), fishModel.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fishModel.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, fishModel.indices.size() * sizeof(unsigned int), fishModel.indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(3);

    // 配置实例化数据
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(FishInstance), (void*)offsetof(FishInstance, position));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);

    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(FishInstance), (void*)offsetof(FishInstance, velocity));
    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1);

    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(FishInstance), (void*)offsetof(FishInstance, scale));
    glEnableVertexAttribArray(6);
    glVertexAttribDivisor(6, 1);

    glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(FishInstance), (void*)offsetof(FishInstance, rotation));
    glEnableVertexAttribArray(7);
    glVertexAttribDivisor(7, 1);

    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(FishInstance), (void*)offsetof(FishInstance, color));
    glEnableVertexAttribArray(8);
    glVertexAttribDivisor(8, 1); // 每个实例使用一个颜色

    glBindVertexArray(0);
}

void FishSimulation::updateFish(float deltaTime) {
    static float time = 0.0f;
    static int currentChunk = 0;       // 当前处理的分区
    const int chunkSize = 200;         // 每次更新的鱼数量
    const float minSpeed = 0.5f;       // 最小速度
    const float floatAmplitude = 2.0f; // 上下浮动的幅度
    const float floatFrequency = 1.0f; // 上下浮动的频率
    const float expansionFactor = 2.0f; // 聚集区域半径扩大倍数
    static bool wasLightTriggered = false; // 用于检测布尔值的变化
    time += deltaTime;

    // 获取 LightingManager 实例和布尔值
    LightingManager& lightingManager = LightingManager::getInstance();
    bool lightTriggered = lightingManager.gettoggleLightingPreset(); // 假设有此方法

    int start = currentChunk * chunkSize;
    int end = std::min(start + chunkSize, numInstances);

    for (int i = start; i < end; ++i) {
        auto& fish = instances[i];

        if (lightTriggered) {

            // 转换为围绕 x=0, z=0 的轴旋转，并向轴聚集
            glm::vec3 targetAxis = glm::vec3(0.0f, fish.position.y, 0.0f); // x=0, z=0 轴
            glm::vec3 directionToAxis = glm::normalize(targetAxis - fish.position);

            // 聚集力：趋向于靠近轴
            glm::vec3 attractionForce = directionToAxis * 0.5f; // 聚集力权重

            // 圆周运动力：围绕轴旋转
            glm::vec3 tangent = glm::cross(directionToAxis, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::vec3 rotationalForce = tangent * 2.0f; // 旋转速度权重

            // 综合运动
            fish.velocity = glm::lerp(fish.velocity, attractionForce + rotationalForce, 0.5f);

            // 更新位置
            fish.position += fish.velocity * deltaTime * 10.0f;
        }
        else {
            // 正常聚集逻辑
            glm::vec3 randomAcceleration = glm::vec3(
                randomFloat(-0.3f, 0.3f),
                randomFloat(-0.3f, 0.3f),
                randomFloat(-0.3f, 0.3f)
            );

            glm::vec3 closestCenter = centers[0];
            float minDistance = glm::length(fish.position - centers[0]);
            for (const auto& center : centers) {
                float distance = glm::length(fish.position - center);
                if (distance < minDistance) {
                    minDistance = distance;
                    closestCenter = center;
                }
            }

            // 扩大聚集区域的半径
            float expandedRadius = minDistance * expansionFactor;

            // 计算聚集力
            glm::vec3 directionToCenter = glm::normalize(closestCenter - fish.position);
            glm::vec3 attractionForce = directionToCenter * glm::clamp(expandedRadius / 15.0f, 0.0f, 1.0f); // 聚集力随扩展距离减弱

            // 更新速度
            fish.velocity = glm::lerp(fish.velocity, attractionForce, 0.5f);
            fish.velocity += randomAcceleration * deltaTime * 0.8f; // 增加速度扰动

            // 加入上下浮动效果
            float individualOffset = static_cast<float>(i) * 0.2f; // 为每条鱼添加一个偏移值
            fish.velocity.y += floatAmplitude * sin(floatFrequency * time + individualOffset);

            // 强制最小速度
            if (glm::length(fish.velocity) < minSpeed) {
                fish.velocity += glm::normalize(fish.velocity) * (minSpeed - glm::length(fish.velocity));
            }

            // 更新位置
            fish.position += fish.velocity * deltaTime;
        }

        // 边界反弹逻辑
        if (fish.position.x > 30.0f * expansionFactor || fish.position.x < -30.0f * expansionFactor) fish.velocity.x *= -1.0f;
        if (fish.position.y > 25.0f * expansionFactor || fish.position.y < 0.0f) fish.velocity.y *= -1.0f;
        if (fish.position.z > 30.0f * expansionFactor || fish.position.z < -30.0f * expansionFactor) fish.velocity.z *= -1.0f;
    }

    // 更新区域索引
    currentChunk = (currentChunk + 1) % ((numInstances + chunkSize - 1) / chunkSize);

    // 更新实例缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, start * sizeof(FishInstance), (end - start) * sizeof(FishInstance), &instances[start]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // 更新布尔值状态
    wasLightTriggered = lightTriggered;
}

// 渲染鱼群
void FishSimulation::renderFish(GLuint shaderProgram) {
    glUseProgram(shaderProgram);
    LightingManager& lighting = LightingManager::getInstance();

    GLint GPUlightDirLoc = glGetUniformLocation(shaderProgram, "lightDirection");
    GLint GPUlightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    GLint GPUsmoothnessLoc = glGetUniformLocation(shaderProgram, "smoothness");

    // 获取点光源参数的 Uniform 位置
    GLint numPointLightsLoc = glGetUniformLocation(shaderProgram, "numPointLights");
    GLint pointLightPositionsLoc = glGetUniformLocation(shaderProgram, "pointLightPositions");
    GLint pointLightColorsLoc = glGetUniformLocation(shaderProgram, "pointLightColors");
    GLint pointLightIntensitiesLoc = glGetUniformLocation(shaderProgram, "pointLightIntensities");
    GLint pointLightRadiiLoc = glGetUniformLocation(shaderProgram, "pointLightRadii");

    // 从 LightingManager 获取点光源数据
    const std::vector<PointLight>& pointLights = LightingManager::getInstance().getPointLights();
    int numPointLights = pointLights.size();

    // 设置点光源数量
    glUniform1i(numPointLightsLoc, numPointLights);

    if (numPointLights > 0) {
        // 准备点光源数据
        std::vector<glm::vec3> pointLightPositions(numPointLights);
        std::vector<glm::vec3> pointLightColors(numPointLights);
        std::vector<float> pointLightIntensities(numPointLights);
        std::vector<float> pointLightRadii(numPointLights);

        for (int i = 0; i < numPointLights; ++i) {
            pointLightPositions[i] = pointLights[i].position;
            pointLightColors[i] = pointLights[i].color;
            pointLightIntensities[i] = pointLights[i].intensity;
            pointLightRadii[i] = pointLights[i].radius;
        }

        // 传递点光源参数给着色器
        glUniform3fv(pointLightPositionsLoc, numPointLights, glm::value_ptr(pointLightPositions[0]));
        glUniform3fv(pointLightColorsLoc, numPointLights, glm::value_ptr(pointLightColors[0]));
        glUniform1fv(pointLightIntensitiesLoc, numPointLights, pointLightIntensities.data());
        glUniform1fv(pointLightRadiiLoc, numPointLights, pointLightRadii.data());
    }


    // 获取雾相关参数的 Uniform 位置
    GLint fogColorLoc = glGetUniformLocation(shaderProgram, "fogColor");
    GLint fogDensityLoc = glGetUniformLocation(shaderProgram, "fogDensity");
    GLint fogHeightStartLoc = glGetUniformLocation(shaderProgram, "fogHeightStart");
    GLint fogHeightEndLoc = glGetUniformLocation(shaderProgram, "fogHeightEnd");
    GLint fogDistanceStartLoc = glGetUniformLocation(shaderProgram, "fogDistanceStart");
    GLint fogDistanceEndLoc = glGetUniformLocation(shaderProgram, "fogDistanceEnd");

    // 获取 LightingManager 的实例
    const LightingManager& lightingManager = LightingManager::getInstance();
    const FogSettings& fogSettings = lightingManager.getFogSettings();

    // 设置雾效参数
    glUniform3fv(fogColorLoc, 1, &fogSettings.fogColor[0]);
    glUniform1f(fogDensityLoc, fogSettings.fogDensity);
    glUniform1f(fogHeightStartLoc, fogSettings.fogHeightStart);
    glUniform1f(fogHeightEndLoc, fogSettings.fogHeightEnd);
    glUniform1f(fogDistanceStartLoc, fogSettings.fogDistanceStart);
    glUniform1f(fogDistanceEndLoc, fogSettings.fogDistanceEnd);

    // 设置默认的光照方向、光照颜色和 smooth 值
    glUniform3fv(GPUlightDirLoc, 1, &lighting.getLightDirection()[0]);
    glUniform3fv(GPUlightColorLoc, 1, &lighting.getLightColor()[0]);
    glUniform1f(GPUsmoothnessLoc, static_cast<float>(lighting.getSmoothness()));
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    // 全局变量：平移矩阵
    glm::vec3 translation(0.0f, 0.0f, 0.0f);
    modelMatrix = glm::translate(modelMatrix, translation);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glm::vec3 viewPos = camera.position;

    // 传递位置
    GLuint viewPosLocation = glGetUniformLocation(shaderProgram, "viewPos");
    glUniform3fv(viewPosLocation, 1, glm::value_ptr(viewPos));
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(1920.0f / 1080.0f);  // 假设窗口是1920.0f / 1080.0f
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // 绑定 Diffuse (Albedo) 贴图到纹理单元 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fishModel.textureID_D);
    glUniform1i(glGetUniformLocation(shaderProgram, "albedoMap"), 0);

    //// 绑定 Normal 贴图到纹理单元 1
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fishModel.textureID_N);
    glUniform1i(glGetUniformLocation(shaderProgram, "normalMap"), 1);

    // 绑定 Roughness 贴图到纹理单元 2
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, fishModel.textureID_R);
    glUniform1i(glGetUniformLocation(shaderProgram, "RoughnessMap"), 2);

    glBindVertexArray(fishModel.VAO);
    glDrawElementsInstanced(GL_TRIANGLES, fishModel.indices.size(), GL_UNSIGNED_INT, 0, numInstances);
    glBindVertexArray(0);
}
