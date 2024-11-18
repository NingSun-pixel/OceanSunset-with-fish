#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;        // 顶点颜色 (用于mask控制)
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 vertColor;

uniform float uTime;                        // 时间变量，用于控制动画
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec3 animatedPosition = aPos;

    float mask = aColor.g;

    // when mask is lower than 1,set it move
    if (mask < 1.0) {
        // calculate the strength
        float swayStrength = (1 - mask) * (1 - mask) * 1;   
        float swayOffset = sin(uTime * 2.0 + aPos.x * 0.5) * swayStrength;

        // X direction
        animatedPosition.x += swayOffset;
    }
    gl_Position = projection * view * model * vec4(animatedPosition.x, animatedPosition.z, animatedPosition.y, 1.0);
    vertColor = aColor;
    TexCoords = aTexCoords;
}