#include "Model.h"
#include "ShaderHandler.h"
#include "ModelLoader.h"
#include "Entity.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

Model::Model(bool renderFromCentrePosition, const glm::vec3& AABBSizeFromCenter, eModelName modelName, const glm::vec3& scale)
	: modelName(modelName),
	renderFromCentrePosition(renderFromCentrePosition),
	AABBSizeFromCenter(AABBSizeFromCenter),
	scale(scale),
	meshes(),
	textures()
{}

std::unique_ptr<Model> Model::create(const std::string & fileName, bool renderFromCentrePosition, 
	const glm::vec3& AABBSizeFromCenter, eModelName modelName, const glm::vec3& scale)
{
	Model* model = new Model(renderFromCentrePosition, AABBSizeFromCenter, modelName, scale);
	if (!ModelLoader::loadModel(fileName, *model))
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
	glm::vec3 modelPosition = position;
	if (renderFromCentrePosition)
	{
		modelPosition.x += AABBSizeFromCenter.x;
		modelPosition.z -= AABBSizeFromCenter.z;
	}
	glm::mat4 model = glm::translate(glm::mat4(1.0), modelPosition);
	model = glm::scale(model, scale);
	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", model);

	for (const auto& mesh : meshes)
	{
		mesh.render(shaderHandler);
	}
}

void Model::render(ShaderHandler& shaderHandler, const Entity& entity) const
{
	glm::vec3 entityPosition = entity.getPosition();
	if (renderFromCentrePosition)
	{
		entityPosition.x += (entity.getAABB().m_right - entity.getAABB().m_left) / 2.0f;
		entityPosition.z -= (entity.getAABB().m_forward - entity.getAABB().m_back) / 2.0f;
	}

	glm::mat4 model = glm::translate(glm::mat4(1.0f), entityPosition);
	model = glm::scale(model, scale);
	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", model);

	for (const auto& mesh : meshes)
	{
		mesh.render(shaderHandler, entity.isSelected());
	}
}