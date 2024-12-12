#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec2 aTexCoords;


out vec2 TexCoords;
out vec3 vertColor;
out vec3 FragPos;
out vec3 Normal;

uniform float uTime;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec3 animatedPosition = aPos;
    float mask = aColor.g;

    // Animation logic for movement based on the mask value
    if (mask < 1.0) {
        float swayStrength = (1 - mask) * (1 - mask) * 1;
        float swayOffset = sin(uTime * 2.0 + aPos.x * 0.5) * swayStrength;
        animatedPosition.x += swayOffset;
    }

    // Calculate position and normal in world space
    FragPos = vec3(model * vec4(animatedPosition, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;

    gl_Position = projection * view * vec4(FragPos.x, FragPos.z, FragPos.y, 1.0);
    vertColor = aColor;
    TexCoords = aTexCoords;
}
