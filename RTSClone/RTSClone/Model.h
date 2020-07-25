#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Mesh.h"
#include <string>
#include <memory>

class ShaderHandler;
struct Model : private NonMovable, private NonCopyable
{
	static std::unique_ptr<Model> create(const std::string& filePath);

	void attachMeshesToVAO() const;
	void render(ShaderHandler& shaderHandler) const;


	std::vector<Mesh> meshes;
    std::vector<MeshTextureDetails> textures;
	
private:
	Model();
};