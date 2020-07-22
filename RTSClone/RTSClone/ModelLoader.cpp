#include "ModelLoader.h"
#include "Model.h"
#include "glad.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <SFML/Graphics.hpp>

void processNode(aiNode* node, const aiScene* scene, Model& model, std::vector<MeshTextureDetails>& loadedTextures, const std::string& directory);
Mesh processMesh(aiMesh* mesh, const aiScene* scene, std::vector<MeshTextureDetails>& loadedTextures, const std::string& directory);
unsigned int TextureFromFile(const char* path, const std::string& directory);
std::vector<MeshTextureDetails> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, 
    std::vector<MeshTextureDetails>& loadedTextures, const std::string& directory);

bool ModelLoader::loadModel(const std::string& filePath, Model& model)
{
    std::vector<MeshTextureDetails> loadedTextures;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << "\n";
        return false;
    }

    std::string directory = filePath.substr(0, filePath.find_last_of('/'));
    processNode(scene->mRootNode, scene, model, loadedTextures, directory);

    return true;
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void processNode(aiNode* node, const aiScene* scene, Model& model, std::vector<MeshTextureDetails>& loadedTextures, const std::string& directory)
{
    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        model.meshes.emplace_back(processMesh(mesh, scene, loadedTextures, directory));
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, model, loadedTextures, directory);
    }
}

Mesh processMesh(aiMesh* mesh, const aiScene* scene, std::vector<MeshTextureDetails>& loadedTextures, const std::string& directory)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<MeshTextureDetails> textures;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;
        // normals
        //vector.x = mesh->mNormals[i].x;
        //vector.y = mesh->mNormals[i].y;
        //vector.z = mesh->mNormals[i].z;
        //vertex.normal = vector;
        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.textCoords = vec;
        }
        else
        {
            vertex.textCoords = glm::vec2(0.0f, 0.0f);
        }
        
        //// tangent
        //vector.x = mesh->mTangents[i].x;
        //vector.y = mesh->mTangents[i].y;
        //vector.z = mesh->mTangents[i].z;
        //vertex.Tangent = vector;
        //// bitangent
        //vector.x = mesh->mBitangents[i].x;
        //vector.y = mesh->mBitangents[i].y;
        //vector.z = mesh->mBitangents[i].z;
        //vertex.Bitangent = vector;

        vertices.push_back(vertex);
    }

    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 1. diffuse maps
    std::vector<MeshTextureDetails> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", loadedTextures, directory);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    //// 2. specular maps
    //std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", loadedTextures, directory);
    //textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    //// 3. normal maps
    //std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", loadedTextures, directory);
    //textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    //// 4. height maps
    //std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height", loadedTextures, directory);
    //textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    // return a mesh object created from the extracted mesh data
    return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
std::vector<MeshTextureDetails> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName,
    std::vector<MeshTextureDetails>& loadedTextures, const std::string& directory)
{
    std::vector<MeshTextureDetails> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for (unsigned int j = 0; j < loadedTextures.size(); j++)
        {
            if (std::strcmp(loadedTextures[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(loadedTextures[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if (!skip)
        {   // if texture hasn't been loaded already, load it
            MeshTextureDetails texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            loadedTextures.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
        }
    }

    return textures;
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