#include "ModelLoader.h"

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

    std::filesystem::path texturePath_D = modelDirectory + "/Texture_DNR/" + modelName  + "_D.png";
    cout << texturePath_D << endl;
    if (std::filesystem::exists(texturePath_D)) {
        model.textureID_D = TextureManager::getTexture(texturePath_D.string().c_str());
    }

    std::filesystem::path texturePath_N = modelDirectory + "/Texture_DNR/" + modelName + "_N.png";
    cout << texturePath_N << endl;
    if (std::filesystem::exists(texturePath_N)) {
        model.textureID_N = TextureManager::getTexture(texturePath_N.string().c_str());
    }

    std::filesystem::path texturePath_R = modelDirectory + "/Texture_DNR/" + modelName + "_R.png";
    cout << texturePath_R << endl;
    if (std::filesystem::exists(texturePath_R)) {
        model.textureID_R = TextureManager::getTexture(texturePath_R.string().c_str());
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

    glBindVertexArray(0);
}




void loadSeparateModels(const std::vector<std::string>& fbxFiles, std::vector<Model>& models) {
    for (const auto& file : fbxFiles) {
        Model model;
        loadSingleModel(file, model);
        models.push_back(model);
    }
}

std::vector<std::string> getAllFBXFiles(const std::string& folderPath) {
    std::vector<std::string> fbxFiles;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(folderPath)) {
        if (entry.path().extension() == ".fbx") {
            fbxFiles.push_back(entry.path().string());
        }
    }

    return fbxFiles;
}

