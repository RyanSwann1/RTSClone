#include "Model.h"
#include "ShaderHandler.h"
#include "ModelLoader.h"
#include "Entity.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

namespace
{
	const std::string MODELS_DIRECTORY = "models/";
}

Model::Model(bool renderFromCentrePosition, const glm::vec3& sizeFromCentre, eModelName modelName)
	: modelName(modelName),
	renderFromCentrePosition(renderFromCentrePosition),
	sizeFromCentre(sizeFromCentre),
	meshes(),
	textures()
{}

std::unique_ptr<Model> Model::create(const std::string & filePath, bool renderFromCentrePosition, 
	const glm::vec3& sizeFromCentre, eModelName modelName)
{
	Model* model = new Model(renderFromCentrePosition, sizeFromCentre, modelName);
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

void Model::render(ShaderHandler& shaderHandler, const glm::vec3& position) const
{
	glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(1.0f, 1.0f, 1.0f));
	model = glm::translate(model, position);
	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", model);

	for (const auto& mesh : meshes)
	{
		mesh.render(shaderHandler);
	}
}

void Model::render(ShaderHandler & shaderHandler, const Entity& entity) const
{
	glm::vec3 entityPosition = entity.getPosition();
	if (renderFromCentrePosition)
	{
		entityPosition.x += (entity.getAABB().m_right - entity.getAABB().m_left) / 2.0f;
		entityPosition.z -= (entity.getAABB().m_forward - entity.getAABB().m_back) / 2.0f;
	}

	glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(1.0f, 1.0f, 1.0f));
	model = glm::translate(model, entityPosition);
	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", model);

	for (const auto& mesh : meshes)
	{
		mesh.render(shaderHandler, entity.isSelected());
	}
}