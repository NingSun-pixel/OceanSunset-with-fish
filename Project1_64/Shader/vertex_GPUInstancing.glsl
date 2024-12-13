#version 330 core

// 顶点属性
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoords;

// 实例化属性
layout(location = 4) in vec3 instancePosition;    // 实例位置
layout(location = 5) in vec3 instanceVelocity;    // 实例速度
layout(location = 6) in vec3 instanceScale;       // 实例缩放
layout(location = 7) in float instanceRotation;   // 实例旋转角度
uniform float deltaTime;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 VertColor;
out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

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

    gl_Position = projection * view * translationMatrix * vec4(inPosition.x, inPosition.z, inPosition.y, 1.0);
    Normal = mat3(transpose(inverse(model))) * inNormal;
    VertColor = updatedPosition; // 设置颜色
    TexCoords = inTexCoords;
}
