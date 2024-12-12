#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
public:
    Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch, float startFov);
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio) const;
    void processKeyboard(char key, float deltaTime);
    void processMouseMovement(float xOffset, float yOffset);
    glm::vec3 position, front, up;

private:
    void updateCameraVectors();
    float yaw, pitch, fov;
};

#endif // CAMERA_H
