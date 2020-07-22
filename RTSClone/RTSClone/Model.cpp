#include "Model.h"
#include "ShaderHandler.h"

Model::Model()
	: meshes(),
	textures()
{}

void Model::attachMeshesToVAO() const
{
	for (const auto& mesh : meshes)
	{
		mesh.attachToVAO();
	}
}

void Model::render(ShaderHandler & shaderHandler) const
{
	for (const auto& mesh : meshes)
	{
		mesh.render(shaderHandler);
	}
}
