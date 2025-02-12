#include "Camera.h"

extern float pitchAngleY;
extern float pitchAngleZ;
extern float pitchAngleX;

extern glm::quat rotationQuat;
extern glm::vec3 startPosition;

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch, float startFov)
    : position(startPosition), up(startUp), yaw(startYaw), pitch(startPitch), fov(startFov) {
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 1000.0f);
}

void Camera::processKeyboard(char key, float deltaTime) {
    float velocity = 25.0f * deltaTime;

    if (key == ' ')
    {
        position = glm::vec3(5.51846,60.0067,74.5267);
        front = glm::vec3(-0.0720887, -0.625243, -0.777094);

    }

    if (key == '0')
    {
        position = glm::vec3(6.1834,16.4986,-1.64565);
        front = glm::vec3(0.980806, -0.194234, -0.0171206);

    }

    // ✅ 先处理位移
    if (key == 'w' || key == 'W')
        position += front * velocity;
    if (key == 's' || key == 'S')
        position -= front * velocity;
    if (key == 'a' || key == 'A')
        position -= glm::normalize(glm::cross(front, up)) * velocity;
    if (key == 'd' || key == 'D')
        position += glm::normalize(glm::cross(front, up)) * velocity;


    bool rotated = false;
    glm::quat qRot(1, 0, 0, 0); // 默认单位四元数

    // ✅ 处理旋转
    if (key == 'Q' || key == 'q') {
        pitchAngleY += 5.0f;
        qRot = glm::angleAxis(glm::radians(5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        rotated = true;
    }
    if (key == 'E' || key == 'e') {
        pitchAngleY -= 5.0f;
        qRot = glm::angleAxis(glm::radians(-5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        rotated = true;
    }
    if (key == 'Z' || key == 'z') {
        pitchAngleZ += 5.0f;
        qRot = glm::angleAxis(glm::radians(5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        rotated = true;
    }
    if (key == 'C' || key == 'c') {
        pitchAngleZ -= 5.0f;
        qRot = glm::angleAxis(glm::radians(-5.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        rotated = true;
    }
    if (key == '1') {
        pitchAngleX += 5.0f;
        qRot = glm::angleAxis(glm::radians(5.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        rotated = true;
    }
    if (key == '3') {
        pitchAngleX -= 5.0f;
        qRot = glm::angleAxis(glm::radians(-5.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        rotated = true;
    }

    // ✅ 只有在旋转按键被按下时才更新四元数
    if (rotated) {
        rotationQuat = qRot * rotationQuat;
        rotationQuat = glm::normalize(rotationQuat);  // 归一化，防止数值漂移
    }
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
