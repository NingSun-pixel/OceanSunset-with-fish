#include "MouseHandler.h"

MouseHandler::MouseHandler(Camera& cameraRef) : camera(cameraRef) {}

void MouseHandler::handleMouseButton(int button, int state, int x, int y) {
    if (!TwEventMouseButtonGLUT(button, state, x, y)) { // 如果事件未被UI处理
        if (button == GLUT_LEFT_BUTTON) {
            if (state == GLUT_DOWN) {
                mousePressed = true;
                lastX = x;
                lastY = y;
            }
            else if (state == GLUT_UP) {
                mousePressed = false;
            }
        }
    }
}

void MouseHandler::handleMouseMotion(int x, int y) {
    if (!TwEventMouseMotionGLUT(x, y)) { // 如果事件未被 UI 处理
        if (mousePressed) {
            float xOffset = x - lastX;
            float yOffset = lastY - y; // 注意 Y 偏移是反向的

            lastX = x;
            lastY = y;

            // 调整视角
            camera.processMouseMovement(xOffset, yOffset);
            glutPostRedisplay();
        }
    }
}

void MouseHandler::processKeys(unsigned char key, int x, int y) {
    float deltaTime = 0.1f;

    camera.processKeyboard(key, deltaTime);

    //if (!TwEventKeyboardGLUT(key, x, y)) { // 如果事件未被UI处理
    //    glutPostRedisplay();
    //}
}

