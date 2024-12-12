#ifndef LIGHTINGMANAGER_H
#define LIGHTINGMANAGER_H

#include <iostream>
#include <glm/glm.hpp>

class LightingManager {
private:
    // 私有构造函数，确保外部无法实例化
    LightingManager()
        : lightDirection(glm::normalize(glm::vec3(-0.55f, -0.55f, -0.66f))),
        lightColor(glm::vec3(1.0f, 1.0f, 1.0f)),
        smoothness(0.6f) {}

    // 禁止拷贝和赋值
    LightingManager(const LightingManager&) = delete;
    LightingManager& operator=(const LightingManager&) = delete;

    // 内部存储的全局变量
    glm::vec3 lightDirection;
    glm::vec3 lightColor;
    glm::float64 smoothness;

public:
    // 获取单例实例
    static LightingManager& getInstance() {
        static LightingManager instance; // 静态局部变量，确保唯一实例
        return instance;
    }

    // Getter 和 Setter 方法
    const glm::vec3& getLightDirection() const { return lightDirection; }
    void setLightDirection(const glm::vec3& direction) { lightDirection = glm::normalize(direction); }

    const glm::vec3& getLightColor() const { return lightColor; }
    void setLightColor(const glm::vec3& color) { lightColor = color; }

    glm::float64 getSmoothness() const { return smoothness; }
    void setSmoothness(glm::float64 value) { smoothness = value; }
};

#endif // LIGHTINGMANAGER_H
