#ifndef LIGHTINGMANAGER_H
#define LIGHTINGMANAGER_H

#include <iostream>
#include <glm/glm.hpp>

struct FogSettings {
    glm::vec3 fogColor = glm::vec3(0.8f, 0.4f, 0.2f);
    float fogDensity = 0.05f;
    float fogHeightStart = 10.0f;
    float fogHeightEnd = 50.0f;
    float fogDistanceStart = 50.0f;
    float fogDistanceEnd = 200.0f;
};

class LightingManager {
private:
    LightingManager()
        : lightDirection(glm::normalize(glm::vec3(-0.55f, -0.55f, -0.66f))),
        lightColor(glm::vec3(1.0f, 0.2f, 0.0f)),
        smoothness(15.0f),
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
    static LightingManager& getInstance() {
        static LightingManager instance;
        return instance;
    }

    // Getter 和 Setter
    const glm::vec3& getLightDirection() const { return lightDirection; }
    const glm::vec3& getLightColor() const { return lightColor; }
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
                targetSmoothness = 15.0;
                targetFogColor = glm::vec3(0.8f, 0.4f, 0.2f);
            }
            else {
                targetLightDirection = glm::vec3(-0.55f, -0.55f, -0.66f);
                targetLightColor = glm::vec3(0.2f, 1.0f, 1.0f);
                targetSmoothness = 6.0;
                targetFogColor = glm::vec3(0.25f, 0.5f, 0.5f);
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
