#include "Camera.h"
#include<iostream>

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch, float startFov)
    : position(startPosition), up(startUp), yaw(startYaw), pitch(startPitch), fov(startFov) {
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
}

void Camera::processKeyboard(char key, float deltaTime) {
    float velocity = 10.0f * deltaTime;
    if (key == 'w' || key == 'W')
        position += front * velocity;
    if (key == 's' || key == 'S')
        position -= front * velocity;
    if (key == 'a' || key == 'A')
        position -= glm::normalize(glm::cross(front, up)) * velocity;
    if (key == 'd' || key == 'D')
        position += glm::normalize(glm::cross(front, up)) * velocity;

}

void Camera::processMouseMovement(float xOffset, float yOffset) {
    yaw += xOffset * 0.1f;
    pitch += yOffset * 0.1f;
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
}
