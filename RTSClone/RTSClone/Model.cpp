#include "Model.h"
#include "ShaderHandler.h"
#include "ModelLoader.h"

namespace
{
	const std::string MODELS_DIRECTORY = "models/";
}

Model::Model(bool renderFromCentrePosition, const glm::vec3& sizeFromCentre)
	: renderFromCentrePosition(renderFromCentrePosition),
	sizeFromCentre(sizeFromCentre),
	meshes(),
	textures()
{}

std::unique_ptr<Model> Model::create(const std::string & filePath, bool renderFromCentrePosition, const glm::vec3& sizeFromCentre)
{
	Model* model = new Model(renderFromCentrePosition, sizeFromCentre);
	if (!ModelLoader::loadModel(MODELS_DIRECTORY + filePath, *model))
	{
		delete model;
		return std::unique_ptr<Model>();
	}
	
	model->attachMeshesToVAO();
	return std::unique_ptr<Model>(model);
}

void Model::attachMeshesToVAO() const
{
	for (const auto& mesh : meshes)
	{
		mesh.attachToVAO();
	}
}

void Model::render(ShaderHandler & shaderHandler, bool selected) const
{
	for (const auto& mesh : meshes)
	{
		mesh.render(shaderHandler, selected);
	}
}