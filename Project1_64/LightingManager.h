#ifndef LIGHTINGMANAGER_H
#define LIGHTINGMANAGER_H

#include <iostream>
#include <glm/glm.hpp>

#define MAX_POINT_LIGHTS 4

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float radius;
};



struct FogSettings {
    glm::vec3 fogColor = glm::vec3(0.27f, 0.44f, 0.47f);
    float fogDensity = 0.05f;
    float fogHeightStart = 10.0f;
    float fogHeightEnd = 50.0f;
    float fogDistanceStart = 70.0f;
    float fogDistanceEnd = 180.0f;
};

class LightingManager {
private:
    LightingManager()
        : lightDirection(glm::normalize(glm::vec3(0.39f, -0.83f, -0.40f))),
        lightColor(glm::vec3(0.2f, 0.74f, 1.0f)),
        smoothness(3.8f),
        toggleLightingPreset(false),
        targetLightDirection(lightDirection),
        targetLightColor(lightColor),
        targetSmoothness(smoothness),
        targetFogColor(fogSettings.fogColor),
        transitionProgress(0.0f),
        transitionDuration(15.0f),
        isTransitioning(false),
        fogSettings() {} // 初始化雾设置

    LightingManager(const LightingManager&) = delete;
    LightingManager& operator=(const LightingManager&) = delete;

    glm::vec3 lightDirection;
    glm::vec3 lightColor;
    glm::float64 smoothness;

    glm::vec3 targetLightDirection;
    glm::vec3 targetLightColor;
    glm::float64 targetSmoothness;

    glm::vec3 targetFogColor;

    float transitionProgress;
    float transitionDuration;
    bool isTransitioning;
    bool toggleLightingPreset;

    FogSettings fogSettings; // 新增雾设置


public:
    std::vector<PointLight> pointLights; // 点光源

    static LightingManager& getInstance() {
        static LightingManager instance;
        return instance;
    }

    // Getter 和 Setter
    const glm::vec3& getLightDirection() const { return lightDirection; }
    const glm::vec3& getLightColor() const { return lightColor; }
    // 获取点光源的可修改引用
    std::vector<PointLight>& getPointLights() { return pointLights; }
    glm::float64 getSmoothness() const { return smoothness; }

    bool gettoggleLightingPreset() const { return toggleLightingPreset; }

    void settoggleLightingPreset(bool value) {
        if (isTransitioning)
            return;

        if (toggleLightingPreset != value) {
            toggleLightingPreset = value;
            transitionProgress = 0.0f;
            isTransitioning = true;

            if (value) {
                targetLightDirection = glm::vec3(-1.0f, -0.28f, 0.08f);
                targetLightColor = glm::vec3(1.0f, 0.2f, 0.1f);
                targetSmoothness = 10.0f;
                targetFogColor = glm::vec3(0.9f, 0.55f, 0.44f);
                glm::vec3 pointLightColor = glm::vec3(0.6f, 1.0f, 1.0f);
                clearPointLights();
                addPointLight(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 10, 15);
                addPointLight(glm::vec3(40.0f, 10.0f, -40.0f), pointLightColor, 10, 15);
                addPointLight(glm::vec3(-40.0f, 10.0f, 40.0f), pointLightColor, 10, 15);

            }
            else {
                targetLightDirection = glm::vec3(0.39f, -0.83f, -0.40f);
                targetLightColor = glm::vec3(0.2f, 0.74f, 1.0f);
                targetSmoothness = 3.8f;
                targetFogColor = glm::vec3(0.27f, 0.44f, 0.47f);
                glm::vec3 pointLightColor = glm::vec3(0.6f, 1.0f, 1.0f);
                clearPointLights();
                addPointLight(glm::vec3(-15.0f, 10.0f, -15.0f), pointLightColor, 30, 15);
                addPointLight(glm::vec3(15.0f, 15.0f, 15.0f), pointLightColor, 30, 15);
                addPointLight(glm::vec3(-20.0f, 20.0f, 20.0f), pointLightColor, 30, 15);
            }
        }
    }

    void setTransitionDuration(float duration) { transitionDuration = duration; }
    float getTransitionDuration() const { return transitionDuration; }

    void setLightDirection(const glm::vec3& direction) {
        if (!isTransitioning) {
            lightDirection = glm::normalize(direction);
            targetLightDirection = lightDirection;
        }
    }

    void setLightColor(const glm::vec3& color) {
        if (!isTransitioning) {
            lightColor = color;
            targetLightColor = lightColor;
        }
    }

    void setSmoothness(glm::float64 value) {
        if (!isTransitioning) {
            smoothness = value;
            targetSmoothness = smoothness;
        }
    }

    // 点光源的管理
    void addPointLight(const glm::vec3& position, const glm::vec3& color, float intensity, float radius) {
        if (pointLights.size() < MAX_POINT_LIGHTS) {
            pointLights.push_back({ position, color, intensity, radius });
        }
        else {
            std::cerr << "Max point lights reached!" << std::endl;
        }
    }

    void clearPointLights() { pointLights.clear(); }



    // Fog Settings Getter 和 Setter
    const FogSettings& getFogSettings() const { return fogSettings; }

    void setFogColor(const glm::vec3& color) { fogSettings.fogColor = color; }
    void setFogDensity(float density) { fogSettings.fogDensity = density; }
    void setFogHeight(float start, float end) {
        fogSettings.fogHeightStart = start;
        fogSettings.fogHeightEnd = end;
    }

    void updateLighting(float deltaTime) {
        if (isTransitioning) {
            transitionProgress += deltaTime / transitionDuration;
            if (transitionProgress >= 1.0f) {
                transitionProgress = 1.0f;
                isTransitioning = false;
            }

            lightDirection = glm::mix(lightDirection, targetLightDirection, transitionProgress);
            lightColor = glm::mix(lightColor, targetLightColor, transitionProgress);
            smoothness = glm::mix(smoothness, targetSmoothness, transitionProgress);
            fogSettings.fogColor = glm::mix(fogSettings.fogColor, targetFogColor, transitionProgress);

        }
    }
};

#endif // LIGHTINGMANAGER_H
