#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <filesystem> 
#include <cmath>
#include "main.h"


// Ĭ�Ϲ��շ������ɫ
glm::vec3 lightDirection = glm::normalize(glm::vec3(-1.0f, 1.0f, 1.0f)); // б���µĹ��շ���
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);  // ��ɫ��
float smoothness = 0.0f;  // Ĭ�ϵ� smooth ֵ

// �����Ǳ���
float pitchAngleY = 0.0f;
float pitchAngleZ = 0.0f;


// �����������
//void keyboardFunc(unsigned char key, int x, int y) {
//    if (key == 'Q' || key == 'q') {
//        pitchAngleY += 5.0f;  // ���Ӹ�����
//    }
//    if (key == 'e' || key == 'E') {
//        pitchAngleY -= 5.0f;  // ���ٸ�����
//    }
//    glutPostRedisplay();  // ������Ⱦ
//}

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 45.0f);
float lastFrame = 0.0f;

// ��������ṹ
struct Joint {
    glm::vec3 position;      // ����λ��
    float rotationAngle;     // ��ǰ��ת�Ƕ�
    glm::mat4 transform;     // ���ձ任����
};

// ��β�����й���
std::vector<Joint> fishTailJoints;

void UpdateFishTail(float time) {
    float frequency = 2.0f;  // �ڶ�Ƶ��
    float maxAmplitude = 15.0f; // ���ڶ��Ƕ�

    // ����ÿ����������ת�Ƕ�
    for (int i = 0; i < fishTailJoints.size(); i++) {
        // ����ÿ�ι�������λƫ��
        float phaseOffset = static_cast<float>(i) * 0.5f;

        // ���㵽ԭ��ľ��룬�����ݾ������ŷ���
        glm::vec3 position = fishTailJoints[i].position;
        float distance = glm::length(position);  // �������
        float scaleFactor = glm::clamp(distance / 10.0f, 1.0f, 1.0f); // �������ӣ�0.1 �� 1 ֮��

        // ʹ�����Ҳ�������ת�Ƕȣ���Ӧ����������
        fishTailJoints[i].rotationAngle = maxAmplitude * scaleFactor * std::sin(frequency * time + phaseOffset);

        // ������ת�任������ y ����ת��
        fishTailJoints[i].transform = glm::rotate(glm::mat4(1.0f), glm::radians(fishTailJoints[i].rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}


void ApplyFishTailTransform() {
    glm::mat4 parentTransform = glm::mat4(1.0f);

    for (int i = 0; i < fishTailJoints.size(); i++) {
        // �ۻ����任
        fishTailJoints[i].transform = parentTransform * fishTailJoints[i].transform;

        // ���¸��任��������һ������
        parentTransform = fishTailJoints[i].transform;
    }
}


void InitializeFishTail() {
    int numJoints = 5;
    fishTailJoints.resize(numJoints);

    // ��ʼ������λ�ã�������β�� z �����죩
    for (int i = 0; i < numJoints; i++) {
        fishTailJoints[i].position = glm::vec3(0.0f, 0.0f, -i * 0.5f); // ÿ�ι������һ������
        fishTailJoints[i].rotationAngle = 0.0f;
        fishTailJoints[i].transform = glm::mat4(1.0f);
    }
}


//used to be not now:ALL TEX ARE SAVE IN THE SAME MODEL NAME FOLDER,go there and find it
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

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

// Vertex Shader (for convenience, it is defined in the main here, but we will be using text files for shaders in future)
// Note: Input to this shader is the vertex positions that we specified for the triangle. 
// Note: gl_Position is a special built-in variable that is supposed to contain the vertex position (in X, Y, Z, W)
// Since our triangle vertices were specified as vec3, we just set W to 1.0.

// ��ȡ�ļ�����Ϊ�ַ���
std::string readShaderFile(const char* filePath) {
    std::ifstream shaderFile;
    std::stringstream shaderStream;

    // ���ļ�
    shaderFile.open(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }

    // ��ȡ�ļ����������ݵ�����
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();

    // �����ļ�������Ϊ�ַ���
    return shaderStream.str();
}

unsigned int createShader(const char* vertexPath, const char* fragmentPath) {
    // ��ȡ�����Ƭ����ɫ���ļ�
    std::string vertexCode = readShaderFile(vertexPath);
    std::string fragmentCode = readShaderFile(fragmentPath);

    const char* vertexShaderSource = vertexCode.c_str();
    const char* fragmentShaderSource = fragmentCode.c_str();

    // �����ͱ��붥����ɫ��
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);     
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    //cout << vertexShaderSource << endl;
    //cout << fragmentShaderSource<< endl;

    // ���������
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // �����ͱ���Ƭ����ɫ��
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // ���������
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // ������ɫ������
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // ������Ӵ���
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // ɾ����ɫ���������Ѿ������ӵ������У�������Ҫ
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    return shaderProgram;
}


// ȫ�ֱ�����ƽ�ƾ���
glm::vec3 translation(0.0f, 0.0f, 0.0f);

unsigned int shaderProgram_use;
unsigned int textureID;
unsigned int skyboxShaderProgram_use;
GLuint skyboxVAO;



struct Bone {
    std::string name;
    glm::mat4 offsetMatrix; // ��ģ�Ϳռ䵽�����ռ��ƫ�ƾ���
    glm::mat4 finalTransformation; // ���ձ任����
    int parentIndex; // ������������
};

struct VertexWeight {
    int boneID;
    float weight;
};


// ȫ�ֱ��������ڴ洢���ģ��
std::vector<Model> models;
std::vector<Model> fishmodels;
vector<std::string> faces;
GLuint cubemapTexture;


// ��������ģ��
void loadModels(const std::vector<std::string>& fbxFiles) {
    //�ȴ�������TexID
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

    // ������պе�VAO��VBO
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



unsigned int VAO, VBO, EBO;


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
    // ��ȡ�������ͼ��ͶӰ����
    // ��ȡ��ǰʱ�䣨�룩
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    // ����ʱ�������������ɫ��
    glUniform1f(glGetUniformLocation(shaderProgram_use, "uTime"), currentTime);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(1920.0f / 1080.0f);  // ���贰����800x800
    renderSkybox(skyboxShaderProgram_use, skyboxVAO, cubemapTexture, view, projection);
    glDepthFunc(GL_LESS);      // �ָ�Ĭ����Ȳ���ģʽ
    glDepthMask(GL_TRUE);       // ����д����Ȼ�����
    //drawing
    // ʹ����ɫ������
    glUseProgram(shaderProgram_use);

    GLint lightDirLoc = glGetUniformLocation(shaderProgram_use, "lightDirection");
    GLint lightColorLoc = glGetUniformLocation(shaderProgram_use, "lightColor");
    GLint smoothnessLoc = glGetUniformLocation(shaderProgram_use, "smoothness");

    // ����Ĭ�ϵĹ��շ��򡢹�����ɫ�� smooth ֵ
    glUniform3fv(lightDirLoc, 1, &lightDirection[0]);
    glUniform3fv(lightColorLoc, 1, &lightColor[0]);
    glUniform1f(smoothnessLoc, smoothness);

    // control camera speed
    float currentFrame = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;



    // transfer view and projection Matrix to shader
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram_use, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram_use, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    //sub animation
    UpdateFishTail(currentTime); // update the tail
    ApplyFishTailTransform();    // apply the tail animation

    float speed = 0.5f;  // speed
    // hierarchy (big animation)
    float translationDistance = speed * currentTime *0.1f;  // ����ƽ�ƾ���
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(translationDistance, 0.0f, 0.0f));

    for (int i = 0; i < fishmodels.size(); i++) {
        glBindVertexArray(fishmodels[i].VAO);

        glm::mat4 modelMatrix = glm::mat4(1.0f);


        //just the tail sub animate
        //if (i == 0)
        //{
        //    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -i * 0.5f)); // 
        //    modelMatrix *= fishTailJoints[i % fishTailJoints.size()].transform; 
        //}
        //else {
                // ���㸩����ת�������� Mesh ����
        //glm::mat4 model = glm::mat4(1.0f);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(pitchAngleY), glm::vec3(0.0f, 1.0f, 0.0f));

        if (fishmodels[i].meshName == "4_Rotor_2_Body_0") {
            float time = glutGet(GLUT_ELAPSED_TIME) / 200.0f; // ��ȡ�������е�ʱ�䣨�룩
            float rotationAngle = time; // ��������ʱ��仯

            // 2. **Χ��Z����ת**
            modelMatrix = glm::rotate(modelMatrix, rotationAngle, glm::vec3(0.0f, 0.0f, 1.0f));
        }

        modelMatrix = glm::translate(modelMatrix, translation);
        //}
        modelMatrix = translationMatrix * modelMatrix;


        glUniformMatrix4fv(glGetUniformLocation(shaderProgram_use, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

        glDrawElements(GL_TRIANGLES, fishmodels[i].indices.size(), GL_UNSIGNED_INT, 0);
    }



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

    glBindVertexArray(0);  // �����
    TwDraw();

    glutSwapBuffers();
    glutPostRedisplay();
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
    }
}

// ��ʼ�����ں� OpenGL ����
void initGL() {
    // ����������ɫΪ��ɫ
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    shaderProgram_use = createShader("../Project1_64/shader/vertex_shader.glsl", "../Project1_64/shader/fragment_shader.glsl");
    skyboxShaderProgram_use = createShader("../Project1_64/shader/vertex_skybox.glsl", "../Project1_64/shader/fragment_skybox.glsl");
    faces = getAllTexFiles("C:/Users/555/Desktop/assignment/CG_Project_1/SkyBoxTexture");
    cubemapTexture = loadCubemap(faces);
    // ������Ȳ���
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

}

int windowWidth = 1920;
int windowHeight = 1080;
bool mousePressed = false;     // if press or not
float lastX, lastY;            // last month position

MouseHandler* mouseHandler = nullptr;

//�������߼�
void processNormalKeys(unsigned char key, int x, int y) {
    if (mouseHandler) {
        mouseHandler->processKeys(key, x, y);
    }
}



void mouseButtonCallback(int button, int state, int x, int y) {
    if (mouseHandler) {
        mouseHandler->handleMouseButton(button, state, x, y);
    }
}

void mouseMotionCallback(int x, int y) {
    if (mouseHandler) {
        mouseHandler->handleMouseMotion(x, y);
    }
}



// ��ʼ�� OpenGL �� AntTweakBar
void initOpenGLAndAntTweakBar() {
    // ��ʼ�� AntTweakBar
    TwInit(TW_OPENGL, NULL);

    // ����һ���µ� Tweak Bar
    TwBar* bar = TwNewBar("Settings");

    // ���� Tweak Bar ��λ�úʹ�С
    // �� UI ���ڴ��ڵ����½�
    int barWidth = 400;  // Tweak Bar �Ŀ��
    int barHeight = 200; // Tweak Bar �ĸ߶�
    TwWindowSize(windowWidth, windowHeight);

    // ��ӹ��շ������
    TwAddVarRW(bar, "Light Direction", TW_TYPE_DIR3F, &lightDirection[0],
        "label='Light Direction' help='Adjust the light direction'");

    // ��ӹ�����ɫ����
    TwAddVarRW(bar, "Light Color", TW_TYPE_COLOR3F, &lightColor[0],
        "label='Light Color' help='Adjust the light color'");


    // ʹ����ʱ�������ò���
    int position[] = { windowWidth - barWidth, windowHeight - barHeight };
    TwSetParam(bar, NULL, "position", TW_PARAM_INT32, 2, position);

    int size[] = { barWidth, barHeight };
    TwSetParam(bar, NULL, "size", TW_PARAM_INT32, 2, size);

    // ���һ���򵥵İ�ť��һ����������� Tweak Bar
    static float value = 0.0f;
    TwAddVarRW(bar, "Value", TW_TYPE_FLOAT, &value, " label='Value' min=0 max=100 step=1 ");
}

void renderSceneUI() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ��Ⱦ��� OpenGL ����...

    // ��Ⱦ AntTweakBar UI
    TwDraw();

    glutSwapBuffers();
    glutPostRedisplay();
}


int main(int argc, char** argv) {
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    // Set up the window


    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Scene Rendering");
    // Tell glut where the display function is

    // ��ʼ�� GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    initGL();
    initOpenGLAndAntTweakBar();

    glutDisplayFunc(renderScene);
    // ���� MouseHandler ����
    MouseHandler mouseHandlerInstance(camera);
    mouseHandler = &mouseHandlerInstance;
    //�������߼�
    glutKeyboardFunc(processNormalKeys);
    //glutKeyboardFunc(keyboardFunc);
    glutMouseFunc(mouseButtonCallback);
    glutMotionFunc(mouseMotionCallback);
    InitializeFishTail();

    std::vector<std::string> fbxfishFiles = getAllFBXFiles("C:/Users/555/Desktop/assignment/CG_Project_1/Anim/FBX_5");

    loadSeparateModels(fbxfishFiles,fishmodels);
    //// ����ģ�Ͳ����� OpenGL ������
    //std::vector<std::string> fbxFiles = getAllFBXFiles("C:/Users/555/Desktop/assignment/CG_Project_1/FBX_3");
    //loadModels(fbxFiles);

    glutMainLoop(); 
    return 0;
}











