#include "ModelLoader.h"
#include "Model.h"
#include "glad.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <iostream>
#include <SFML/Graphics.hpp>

namespace
{
    const std::string MODELS_DIRECTORY = "../Shared/models/";
}

void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes, std::vector<MeshTextureDetails>& loadedTextures, const std::string& directory);
Mesh processMesh(aiMesh* mesh, const aiScene* scene, std::vector<MeshTextureDetails>& loadedTextures, const std::string& directory);
unsigned int TextureFromFile(const char* path, const std::string& directory);
void loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, std::vector<MeshTextureDetails>& loadedTextures, 
    const std::string& directory, std::vector<MeshTextureDetails>& meshTextureDetails);
Material loadMaterial(aiMaterial* mat);

bool ModelLoader::loadModel(const std::string& fileName, std::vector<Mesh>& meshes)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(MODELS_DIRECTORY + fileName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << "\n";
        return false;
    }

    std::string directory = (MODELS_DIRECTORY + fileName).substr(0, (MODELS_DIRECTORY + fileName).find_last_of('/'));
    std::vector<MeshTextureDetails> loadedTextures;
    processNode(scene->mRootNode, scene, meshes, loadedTextures, directory);

    return true;
}

void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes, std::vector<MeshTextureDetails>& loadedTextures, const std::string& directory)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.emplace_back(processMesh(mesh, scene, loadedTextures, directory));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, meshes, loadedTextures, directory);
    }
}

Mesh processMesh(aiMesh* mesh, const aiScene* scene, std::vector<MeshTextureDetails>& loadedTextures, const std::string& directory)
{
    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<size_t>(mesh->mNumVertices));
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        glm::vec3 position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        glm::vec3 normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        if (mesh->mTextureCoords[0]) 
        {
            vertices.emplace_back(position, normal, glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
        }
        else
        {
            vertices.emplace_back(position, normal, glm::vec2(0.0f, 0.0f));
        }
    }

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {

        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    std::vector<MeshTextureDetails> textures;
    loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", loadedTextures, directory, textures);

    return Mesh(std::move(vertices), std::move(indices), std::move(textures), loadMaterial(material));
}

void loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName, std::vector<MeshTextureDetails>& loadedTextures, 
    const std::string& directory, std::vector<MeshTextureDetails>& meshTextureDetails)
{
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < loadedTextures.size(); j++)
        {
            if (std::strcmp(loadedTextures[j].path.data(), str.C_Str()) == 0)
            {
                meshTextureDetails.push_back(loadedTextures[j]);
                skip = true; 
                break;
            }
        }
        if (!skip)
        { 
            unsigned int ID = TextureFromFile(str.C_Str(), directory);
            meshTextureDetails.emplace_back(ID, typeName, str.C_Str());
            loadedTextures.emplace_back(ID, typeName, str.C_Str());
        }
    }
}

Material loadMaterial(aiMaterial* mat) 
{
    aiColor3D color(0.f, 0.f, 0.f);
    mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    aiString materialName;
    mat->Get(AI_MATKEY_NAME, materialName);

    return Material({ color.r, color.g, color.b }, materialName.C_Str());
}

unsigned int TextureFromFile(const char* path, const std::string& directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    sf::Image image;
    bool textureLoaded = image.loadFromFile(filename);
    if (!textureLoaded)
    {
        std::cout << "Failed to load texture: " << filename << "\n";
    }
    else
    {
        image.flipVertically();

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    return textureID;
}