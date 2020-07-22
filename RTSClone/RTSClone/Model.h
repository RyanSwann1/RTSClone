#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Mesh.h"
#include <string>

class ShaderHandler;
struct Model : private NonMovable, private NonCopyable
{
	Model();

	void attachMeshesToVAO() const;
	void render(ShaderHandler& shaderHandler) const;

	std::vector<Mesh> meshes;
    std::vector<MeshTextureDetails> textures;
};