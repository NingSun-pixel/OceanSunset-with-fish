#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 3) in vec3 instancePosition;
layout(location = 4) in vec3 instanceVelocity;
layout(location = 5) in vec3 instanceScale;
layout(location = 6) in float instanceRotation;
uniform float deltaTime;
uniform mat4 view;
uniform mat4 projection;

out vec3 VertColor;

void main() {
    // 更新鱼的位置
    vec3 updatedPosition = instancePosition + instanceVelocity * deltaTime;

    // 旋转变换
    float angle = radians(instanceRotation);
    mat4 rotationMatrix = mat4(
        vec4(cos(angle), 0.0, sin(angle), 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(-sin(angle), 0.0, cos(angle), 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );

    // 缩放变换
    mat4 scaleMatrix = mat4(
        vec4(instanceScale.x, 0.0, 0.0, 0.0),
        vec4(0.0, instanceScale.y, 0.0, 0.0),
        vec4(0.0, 0.0, instanceScale.z, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );

    // 平移变换
    mat4 translationMatrix = mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(updatedPosition, 1.0)
    );


    mat4 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

    gl_Position = projection * view  * vec4(inPosition, 1.0);
    VertColor = vec3(1.0, 0.5, 0.3); // 设置颜色
}
