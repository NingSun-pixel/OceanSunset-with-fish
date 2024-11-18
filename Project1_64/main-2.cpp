#include<assimp/Importer.hpp>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <fstream>
#include <sstream>
#include <filesystem> 
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

class TextureManager {
public:
    // 获取纹理ID，如果纹理已加载，则返回已有的ID；否则加载新纹理
    static GLuint getTexture(const std::string& texturePath) {
        auto it = textureCache.find(texturePath);
        if (it != textureCache.end()) {
            // 纹理已存在，返回已有的ID
            return it->second;
        }
        else {
            unsigned int textureID;
            glGenTextures(1, &textureID);

            int width, height, nrComponents;
            unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &nrComponents, 0);
            if (data) {
                GLenum format;
                if (nrComponents == 1)
                    format = GL_RED;
                else if (nrComponents == 3)
                    format = GL_RGB;
                else if (nrComponents == 4)
                    format = GL_RGBA;

                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                stbi_image_free(data);
            }
            else {
                std::cout << "Texture failed to load at path: " << texturePath << std::endl;
                stbi_image_free(data);
            }

            textureCache[texturePath] = textureID;
            return textureID;

        }
    }

    // 清理所有加载的纹理
    static void cleanup() {
        for (auto& texture : textureCache) {
            glDeleteTextures(1, &texture.second);
        }
        textureCache.clear();
    }

private:
    static std::unordered_map<std::string, GLuint> textureCache;
};

// 定义静态成员
std::unordered_map<std::string, GLuint> TextureManager::textureCache;


class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    float yaw;
    float pitch;
    float fov;

    Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch, float startFov)
        : position(startPosition), up(startUp), yaw(startYaw), pitch(startPitch), fov(startFov) {
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 getProjectionMatrix(float aspectRatio) const {
        return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
    }

    void processKeyboard(char key, float deltaTime) {
        float velocity = 2.5f * deltaTime;
        if (key == 'w')
            position += front * velocity;
        if (key == 's')
            position -= front * velocity;
        if (key == 'a')
            position -= glm::normalize(glm::cross(front, up)) * velocity;
        if (key == 'd')
            position += glm::normalize(glm::cross(front, up)) * velocity;
    }

    void processMouseMovement(float xOffset, float yOffset) {
        float sensitivity = 0.1f;
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        yaw += xOffset;
        pitch += yOffset;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        updateCameraVectors();
    }

private:
    void updateCameraVectors() {
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(newFront);
    }
};


Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 45.0f);
float lastFrame = 0.0f;

//ALL TEX ARE SAVE IN THE SAME MODEL NAME FOLDER,go there and find it
//所有贴图文件放在当前模型文件同路径的Tex文件夹下，名字和模型文件相同，自动去找
unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

GLuint loadCubemap(std::vector<std::string> faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (GLuint i = 0; i < faces.size(); i++) {
        //get 6 Texture
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}



// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

// Vertex Shader (for convenience, it is defined in the main here, but we will be using text files for shaders in future)
// Note: Input to this shader is the vertex positions that we specified for the triangle. 
// Note: gl_Position is a special built-in variable that is supposed to contain the vertex position (in X, Y, Z, W)
// Since our triangle vertices were specified as vec3, we just set W to 1.0.

// 读取文件内容为字符串
std::string readShaderFile(const char* filePath) {
    std::ifstream shaderFile;
    std::stringstream shaderStream;

    // 打开文件
    shaderFile.open(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }

    // 读取文件缓冲区内容到流中
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();

    // 返回文件内容作为字符串
    return shaderStream.str();
}

unsigned int createShader(const char* vertexPath, const char* fragmentPath) {
    // 读取顶点和片段着色器文件
    std::string vertexCode = readShaderFile(vertexPath);
    std::string fragmentCode = readShaderFile(fragmentPath);

    const char* vertexShaderSource = vertexCode.c_str();
    const char* fragmentShaderSource = fragmentCode.c_str();

    // 创建和编译顶点着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    //cout << vertexShaderSource << endl;
    //cout << fragmentShaderSource<< endl;

    // 检查编译错误
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // 创建和编译片段着色器
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // 检查编译错误
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // 链接着色器程序
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 检查链接错误
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // 删除着色器，它们已经被链接到程序中，不再需要
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    return shaderProgram;
}


// 全局变量：平移矩阵
glm::vec3 translation(0.0f, 0.0f, 0.0f);


// 键盘输入回调函数
//void processNormalKeys(unsigned char key, int x, int y) {
//    float moveSpeed = 0.1f;
//
//    switch (key) {
//    case 'w':  // 向上移动
//        translation.y += moveSpeed;
//        break;
//    case 's':  // 向下移动
//        translation.y -= moveSpeed;
//        break;
//    case 'a':  // 向左移动
//        translation.x -= moveSpeed;
//        break;
//    case 'd':  // 向右移动
//        translation.x += moveSpeed;
//        break;
//    case 'q':  // 向前移动
//        translation.z -= moveSpeed;
//        break;
//    case 'e':  // 向后移动
//        translation.z += moveSpeed;
//        break;
//    case 27:   // ESC 键退出
//        exit(0);
//    }
//
//    // 重新绘制场景
//    glutPostRedisplay();
//}


// 一个简单的结构体来存储顶点信息
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

//std::vector<Vertex> vertices;
//std::vector<unsigned int> indices;
unsigned int shaderProgram_use;
unsigned int textureID;
unsigned int skyboxShaderProgram_use;
GLuint skyboxVAO;


struct Model {
    unsigned int VAO, VBO, EBO;
    unsigned int textureID;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

// 全局变量，用于存储多个模型
std::vector<Model> models;
std::vector<string> ConstNameTexOfFBX = { "CoralRock","Coral_1","Coral5", "Coral9", "Coral1"};
vector<std::string> faces;
GLuint cubemapTexture;
//fbx->Tex
//void loadTextureForModel(const std::string& modelPath, Model& model) {
//    // 获取文件名
//    std::filesystem::path modelFilePath(modelPath);
//    std::string modelName = modelFilePath.stem().string(); // 获取文件名（不带扩展名）
//    cout << modelName;
//    if (modelName.find("CoralRock"))
//    {
//        cout << modelName;
//        modelName = "CoralRock";
//    }
//    else {
//        // 查找点并截取主名称
//        size_t dotPos = modelName.find('.');
//        if (dotPos != std::string::npos) {
//            modelName = modelName.substr(0, dotPos); // 提取点前的主名称
//        }
//    }
//    // 构造纹理路径，假设纹理位于模型文件上级目录中的 Texture 文件夹
//    std::filesystem::path modelDirectory = modelFilePath.parent_path().parent_path();
//    std::filesystem::path texturePath = modelDirectory / "Texture_2" / (modelName + ".png");
//
//    // 加载纹理
//    if (std::filesystem::exists(texturePath)) {
//        model.textureID = loadTexture(texturePath.string().c_str());
//    }
//    else {
//        std::cerr << "Texture file not found at path: " << texturePath << std::endl;
//    }
//}

// 加载单个模型并创建其 VAO、VBO、EBO
void loadSingleModel(const std::string& path, Model& model) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error: " << importer.GetErrorString() << std::endl;
        return;
    }

    aiMesh* mesh = scene->mMeshes[0];

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        vertex.texCoords = mesh->mTextureCoords[0] ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : glm::vec2(0.0f, 0.0f);

        model.vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            model.indices.push_back(face.mIndices[j]);
    }

    std::filesystem::path modelFilePath(path);
    std::string modelDirectory = modelFilePath.parent_path().parent_path().string();
    std::string modelName = modelFilePath.stem().string();

    cout << modelName << endl;
    bool isConstTex = false;

    size_t pos = modelName.find('_');
    if (pos != std::string::npos) {
        string result = modelName.substr(0, pos);
        modelName = result;
    }

    std::filesystem::path texturePath = modelDirectory + "/Texture_2/" + modelName + ".png";
    cout << texturePath << endl;
    if (std::filesystem::exists(texturePath)) {
        model.textureID = TextureManager::getTexture(texturePath.string().c_str());
    }

    glGenVertexArrays(1, &model.VAO);
    glGenBuffers(1, &model.VBO);
    glGenBuffers(1, &model.EBO);

    glBindVertexArray(model.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, model.VBO);
    glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(Vertex), &model.vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(unsigned int), &model.indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

// 加载所有模型
void loadModels(const std::vector<std::string>& fbxFiles) {
    //先创建共享TexID
    for (const auto& file : fbxFiles) {
        Model model;
        loadSingleModel(file, model);
        models.push_back(model);
    }

    //load Skybox
    float skyboxVertices[] = {
        // back
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        // front
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        // left
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,

        // right
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,

         // up
         -1.0f,  1.0f,  1.0f,
         -1.0f,  1.0f, -1.0f,
          1.0f,  1.0f, -1.0f,
          1.0f,  1.0f, -1.0f,
          1.0f,  1.0f,  1.0f,
         -1.0f,  1.0f,  1.0f,

         // down
         -1.0f, -1.0f, -1.0f,
         -1.0f, -1.0f,  1.0f,
          1.0f, -1.0f,  1.0f,
          1.0f, -1.0f,  1.0f,
          1.0f, -1.0f, -1.0f,
         -1.0f, -1.0f, -1.0f
    };

    // 生成天空盒的VAO和VBO
    GLuint skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}


// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    // create a shader object
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }
    // Bind the source code to the shader, this happens before compilation
    glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderText, NULL);
    // compile the shader and check for errors
    glCompileShader(ShaderObj);
    GLint success;
    // check for shader related errors using glGetShaderiv
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }
    // Attach the compiled shader object to the program object
    glAttachShader(ShaderProgram, ShaderObj);
}

#pragma endregion SHADER_FUNCTIONS

void display() {

    glClear(GL_COLOR_BUFFER_BIT);
    // NB: Make the call to draw the geometry in the currently activated vertex buffer. This is where the GPU starts to work!
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glutSwapBuffers();
}


unsigned int VAO, VBO, EBO;

void renderSkybox(GLuint skyboxShader, GLuint skyboxVAO, GLuint cubemapTexture, glm::mat4 view, glm::mat4 projection) {
    // 调整深度测试，使天空盒始终显示在背景
    glDepthFunc(GL_LEQUAL);  // 设置深度函数，允许深度小于或等于的部分通过测试
    glDepthMask(GL_FALSE);   // 禁止写入深度缓冲区

    // 使用天空盒着色器
    glUseProgram(skyboxShader);

    // 设置view和projection矩阵
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view)); // 移除平移分量，只保留旋转
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, glm::value_ptr(skyboxView));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // 绑定天空盒的立方体贴图
    glBindVertexArray(skyboxVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    // 绘制天空盒
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // 恢复深度缓冲区的设置
    glDepthMask(GL_TRUE);    // 恢复深度写入
    glDepthFunc(GL_LESS);    // 恢复深度测试
}

//get all Tex
std::vector<std::string> getAllTexFiles(const std::string& folderPath) {
    std::vector<std::string> fbxFiles;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(folderPath)) {
        if (entry.path().extension() == ".png") {
            fbxFiles.push_back(entry.path().string());
        }
    }

    return fbxFiles;
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // 获取相机的视图和投影矩阵
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(1080.0f / 1080.0f);  // 假设窗口是800x800
    renderSkybox(skyboxShaderProgram_use, skyboxVAO, cubemapTexture, view, projection);
    glDepthFunc(GL_LESS);      // 恢复默认深度测试模式
    glDepthMask(GL_TRUE);       // 允许写入深度缓冲区
    //drawing
    // 使用着色器程序
    glUseProgram(shaderProgram_use);

    // 计算帧时间以控制相机移动速度
    float currentFrame = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;



    // 传递视图和投影矩阵给着色器
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram_use, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram_use, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


    for (const auto& model : models) {
        glBindVertexArray(model.VAO);

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, translation);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram_use, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, model.textureID);
        glUniform1i(glGetUniformLocation(shaderProgram_use, "ourTexture"), 0);

        glDrawElements(GL_TRIANGLES, model.indices.size(), GL_UNSIGNED_INT, 0);
    }

    //vector<std::string> faces = getAllTexFiles("C:/Users/555/Desktop/assignment/CG_Project_1/SkyBoxTexture");

    //renderSkybox(skyboxShaderProgram_use, skyboxVAO, loadCubemap(faces), view, projection);

    glutSwapBuffers();

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
    }
}

// 初始化窗口和 OpenGL 环境
void initGL() {
    // 设置清屏颜色为黑色
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    shaderProgram_use = createShader("../Project1_64/shader/vertex_shader.glsl", "../Project1_64/shader/fragment_shader.glsl");
    skyboxShaderProgram_use = createShader("../Project1_64/shader/vertex_skybox.glsl", "../Project1_64/shader/fragment_skybox.glsl");
    faces = getAllTexFiles("C:/Users/555/Desktop/assignment/CG_Project_1/SkyBoxTexture");
    cubemapTexture = loadCubemap(faces);
    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

}

//get all FBX
std::vector<std::string> getAllFBXFiles(const std::string& folderPath) {
    std::vector<std::string> fbxFiles;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(folderPath)) {
        if (entry.path().extension() == ".fbx") {
            fbxFiles.push_back(entry.path().string());
        }
    }

    return fbxFiles;
}



bool mousePressed = false;     // 是否按下鼠标
float lastX, lastY;            // 上一次鼠标位置

void processNormalKeys(unsigned char key, int x, int y) {
    float deltaTime = 0.1f;
    camera.processKeyboard(key, deltaTime);
    glutPostRedisplay();
}

void mouseButtonCallback(int button, int state, int x, int y) {
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


void mouseMotionCallback(int x, int y) {
    if (mousePressed) {
        float xOffset = x - lastX;
        float yOffset = lastY - y;  // 注意 y 是倒置的

        lastX = x;
        lastY = y;

        // 根据偏移量调整相机视角
        camera.processMouseMovement(xOffset, yOffset);
        glutPostRedisplay();
    }
}




int main(int argc, char** argv) {
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    // Set up the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1080, 1080);
    glutCreateWindow("Scene Rendering");
    // Tell glut where the display function is
    glutDisplayFunc(display);

    glewInit();
    initGL();

    // 加载模型并设置 OpenGL 缓冲区
    std::vector<std::string> fbxFiles = getAllFBXFiles("C:/Users/555/Desktop/assignment/CG_Project_1/FBX_2");
    //std::vector<std::string> fbxFiles;
    //fbxFiles.push_back("C:/Users/555/Desktop/assignment/CG_Project_1/FBX/CoralRock1.001.fbx"); // 替换为您要测试的具体文件路径
    loadModels(fbxFiles);
    //loadModel("C:/Users/555/Desktop/assignment/CG_Project_1/textest.fbx");
    //std::cout << "Vertices count: " << vertices.size() << std::endl;
    //std::cout << "Indices count: " << indices.size() << std::endl;
    //cout << shaderProgram_use;
    ////setupMesh();

    glutDisplayFunc(renderScene);

    glutKeyboardFunc(processNormalKeys);
    glutMouseFunc(mouseButtonCallback);          // 鼠标按下/释放回调
    glutMotionFunc(mouseMotionCallback);         // 鼠标拖动回调

    glutMainLoop();
    return 0;
}











