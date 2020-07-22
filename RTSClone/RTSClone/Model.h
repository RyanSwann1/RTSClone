#pragma once

#include "Mesh.h"
#include <string>

class ShaderHandler;
struct Model
{
	Model();

	void attachMeshesToVAO();
	void render(ShaderHandler& shaderHandler) const;
	

	std::vector<Mesh> meshes;
    std::vector<TextureDetails> textures;
};