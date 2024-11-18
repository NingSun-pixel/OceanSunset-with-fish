#version 330 core

in vec2 TexCoords;
in vec3 vertColor;
out vec4 FragColor;
uniform sampler2D ourTexture;

//问题在这里：TexCoords，UV没传过来(stl没有UV信息)
void main() {
    //FragColor = vec4(vertColor, 1.0); // 设置为红色
    FragColor = texture(ourTexture, TexCoords);
    if (FragColor.a < 0.5)
    {
        discard; // 丢弃片段
    }
}
