#include "FishSimulation.h"
#include <random>

// 辅助函数：生成随机数
float randomFloat(float min, float max) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

// 构造函数
FishSimulation::FishSimulation(int numInstances, Camera& camera)
    : numInstances(numInstances), camera(camera) {}

// 析构函数
FishSimulation::~FishSimulation() {
    glDeleteBuffers(1, &instanceVBO);
}

// 初始化鱼群实例
void FishSimulation::initFishInstances() {
    instances.resize(numInstances);

    for (int i = 0; i < numInstances; ++i) {
        instances[i].position = glm::vec3(
            randomFloat(-10.0f, 10.0f),
            randomFloat(-5.0f, 5.0f),
            randomFloat(-10.0f, 10.0f)
        );
        instances[i].velocity = glm::vec3(
            randomFloat(-0.1f, 0.1f),
            randomFloat(-0.1f, 0.1f),
            randomFloat(-0.1f, 0.1f)
        );
        instances[i].scale = glm::vec3(randomFloat(0.5f, 1.5f));
        instances[i].rotation = randomFloat(0.0f, 360.0f);
    }

    for (const auto& fish : instances) {
        std::cout << "Instance Position: "
            << fish.position.x << ", "
            << fish.position.y << ", "
            << fish.position.z << std::endl;
    }

    // 创建实例缓冲对象
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(FishInstance), &instances[0], GL_DYNAMIC_DRAW);
}

// 加载鱼模型
void FishSimulation::loadFishModels(const std::vector<std::string>& fbxFiles) {
    loadSeparateGPUModels(fbxFiles, fishModels, instances); // 使用 ModelLoader 的加载功能
    if (fishModels.empty()) {
        std::cerr << "Failed to load any fish models!" << std::endl;
    }
}

// 更新鱼群位置
void FishSimulation::updateFish(float deltaTime) {
    for (auto& fish : instances) {
        fish.position += fish.velocity * deltaTime;

        // 简单的边界反弹逻辑
        if (fish.position.x > 10.0f || fish.position.x < -10.0f)
            fish.velocity.x *= -1.0f;
        if (fish.position.y > 5.0f || fish.position.y < -5.0f)
            fish.velocity.y *= -1.0f;
        if (fish.position.z > 10.0f || fish.position.z < -10.0f)
            fish.velocity.z *= -1.0f;
    }

    // 更新缓冲区数据
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instances.size() * sizeof(FishInstance), &instances[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// 渲染鱼群
void FishSimulation::renderFish(GLuint shaderProgram) {
    glUseProgram(shaderProgram);

    // 设置相机
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    for (const auto& model : fishModels) {
        glBindVertexArray(model.VAO);
        //instance
        glDrawElementsInstanced(GL_TRIANGLES, model.indices.size(), GL_UNSIGNED_INT, 0, instances.size());
        if (glIsTexture(model.textureID) == GL_FALSE) {
            std::cerr << "Invalid texture ID!" << std::endl;
        }
        else {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, model.textureID);
            glUniform1i(glGetUniformLocation(shaderProgram, "albedoMap"), 0);
        }
        glBindVertexArray(0);
    }
}

void FishSimulation::loadSingleGPUModel(const std::string& path, Model& model, FishInstance instance) {
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
        // vertex color
        if (mesh->mColors[0]) { // check if have vertex color
            vertex.color = glm::vec3(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b);
        }
        else {
            vertex.color = glm::vec3(1.0f, 1.0f, 1.0f); // no color just white
        }

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

    std::filesystem::path texturePath = modelDirectory + "/Texture_3/" + modelName + ".png";
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

    // vertex position -> location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // vertex color -> location 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    // vertex normal -> location 2
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);

    // UV -> location 3
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    glEnableVertexAttribArray(3);

    // 设置 instancePosition -> location 4
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(FishInstance), (void*)offsetof(FishInstance, position));
    glVertexAttribDivisor(4, 1); // 每个实例调用一次

    // 设置 instanceVelocity -> location 5
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(FishInstance), (void*)offsetof(FishInstance, velocity));
    glVertexAttribDivisor(5, 1);

    // 设置 instanceScale -> location 6
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(FishInstance), (void*)offsetof(FishInstance, scale));
    glVertexAttribDivisor(6, 1);

    // 设置 instanceRotation -> location 7
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, sizeof(FishInstance), (void*)offsetof(FishInstance, rotation));
    glVertexAttribDivisor(7, 1);

    glBindVertexArray(0); // 解绑 VAO
}

void FishSimulation::loadSeparateGPUModels(const std::vector<std::string>& fbxFiles, std::vector<Model>& models, std::vector<FishInstance> instances) {
    //现在只取fbx中的第一个
    for (const auto& instance : instances) {
        Model model;
        loadSingleGPUModel(fbxFiles[0], model, instance);
        models.push_back(model);
    }
    //for (const auto& file : fbxFiles) {
    //    Model model;
    //    loadSingleGPUModel(file, model, instances);
    //    models.push_back(model);
    //}
}