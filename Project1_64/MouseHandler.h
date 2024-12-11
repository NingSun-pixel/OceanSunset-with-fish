#ifndef MOUSE_HANDLER_H
#define MOUSE_HANDLER_H

#include <GL/freeglut.h>
#include <iostream>
#include <AntTweakBar.h>
#include "Camera.h"

class MouseHandler {
public:
    MouseHandler(Camera& camera);

    void handleMouseButton(int button, int state, int x, int y);
    void handleMouseMotion(int x, int y);
    void processKeys(unsigned char key, int x, int y);

private:
    Camera& camera; // 引用相机对象
    bool mousePressed = false;
    float lastX = 0.0f;
    float lastY = 0.0f;
};

#endif // MOUSE_HANDLER_H
