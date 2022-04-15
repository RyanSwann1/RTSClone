#include "ModelLoader.h"
#include "Model.h"
#include "glad.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <SFML/Graphics.hpp>

namespace
{
    const std::string MODELS_DIRECTORY = "../Shared/models/";
}

void processNode(aiNode& node, const aiScene& scene, std::vector<Mesh>& meshes, const std::string& directory);
Mesh processMesh(aiMesh& mesh, const aiScene& scene, const std::string& directory);
Material loadMaterial(aiMaterial& mat);

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
    processNode(*scene->mRootNode, *scene, meshes, directory);

    return true;
}

void processNode(aiNode& node, const aiScene& scene, std::vector<Mesh>& meshes, const std::string& directory)
{
    for (unsigned int i = 0; i < node.mNumMeshes; i++)
    {
        meshes.emplace_back(processMesh(*scene.mMeshes[node.mMeshes[i]], scene, directory));
    }

    for (unsigned int i = 0; i < node.mNumChildren; i++)
    {
        processNode(*node.mChildren[i], scene, meshes, directory);
    }
}

Mesh processMesh(aiMesh& mesh, const aiScene& scene, const std::string& directory)
{
    std::vector<Vertex> vertices;
    vertices.reserve(static_cast<size_t>(mesh.mNumVertices));
    for (unsigned int i = 0; i < mesh.mNumVertices; i++)
    {
        vertices.emplace_back(glm::vec3(mesh.mVertices[i].x, mesh.mVertices[i].y, mesh.mVertices[i].z), 
            glm::vec3(mesh.mNormals[i].x, mesh.mNormals[i].y, mesh.mNormals[i].z));
    }

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < mesh.mNumFaces; i++)
    {
        aiFace face = mesh.mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    return Mesh(std::move(vertices), std::move(indices), loadMaterial(*scene.mMaterials[mesh.mMaterialIndex]));
}

Material loadMaterial(aiMaterial& mat) 
{
    aiColor3D color(0.f, 0.f, 0.f);
    mat.Get(AI_MATKEY_COLOR_DIFFUSE, color);
    aiString materialName;
    mat.Get(AI_MATKEY_NAME, materialName);

    return Material({ color.r, color.g, color.b }, materialName.C_Str());
}