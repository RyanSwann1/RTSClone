#include "Model.h"
#include "ShaderHandler.h"
#include "ModelLoader.h"
#include "Globals.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#ifdef GAME
#include "Entity.h"
#endif // GAME

Model::Model(bool renderFromCentrePosition, const glm::vec3& AABBSizeFromCenter, const glm::vec3& scale,
	const std::string& fileName, std::vector<Mesh>&& meshes)
	: modelName(fileName),
	renderFromCentrePosition(renderFromCentrePosition),
	AABBSizeFromCenter(AABBSizeFromCenter),
	scale(scale),
	meshes(std::move(meshes))
{
	attachMeshesToVAO();
}

void Model::render(ShaderHandler & shaderHandler, glm::vec3 position, bool highlight, const glm::vec3& rotation) const
{
	setModelMatrix(shaderHandler, position, highlight, rotation);

	for (const auto& mesh : meshes)
	{
		mesh.render(shaderHandler, highlight);
	}
}

void Model::render(ShaderHandler& shaderHandler, glm::vec3 position, bool highlight, const glm::vec3& rotation, 
	eFactionController owningFactionController) const
{
	setModelMatrix(shaderHandler, position, highlight, rotation);

	for (const auto& mesh : meshes)
	{
		mesh.render(shaderHandler, owningFactionController, highlight);
	}
}

void Model::attachMeshesToVAO() const
{
	for (const auto& mesh : meshes)
	{
		mesh.attachToVAO();
	}
}

void Model::setModelMatrix(ShaderHandler& shaderHandler, glm::vec3 position, bool highlight, const glm::vec3& rotation) const
{
	glm::mat4 model = glm::mat4(1.0f);
	if (renderFromCentrePosition)
	{
		position.x += AABBSizeFromCenter.x;
		position.z -= AABBSizeFromCenter.z;

		model = glm::translate(model, position);
		model = glm::scale(model, scale);
		model = glm::translate(model, glm::vec3(-AABBSizeFromCenter.x, 0.0f, AABBSizeFromCenter.z));
		model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(AABBSizeFromCenter.x, 0.0f, -AABBSizeFromCenter.z));
	}
	else
	{
		model = glm::translate(model, position);
		model = glm::scale(model, scale);
		model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", model);
}

std::unique_ptr<Model> Model::create(const std::string & fileName, bool renderFromCentrePosition, 
	const glm::vec3& AABBSizeFromCenter, const glm::vec3& scale)
{
	std::vector<Mesh> meshes;
	if (!ModelLoader::loadModel(fileName, meshes))
	{
		return std::unique_ptr<Model>();
	}

	return std::unique_ptr<Model>(new Model(renderFromCentrePosition, AABBSizeFromCenter, scale, fileName, std::move(meshes)));
}

void Model::render(ShaderHandler& shaderHandler, const glm::vec3& position, glm::vec3 rotation) const
{	
	render(shaderHandler, position, false, rotation);
}

#ifdef GAME
void Model::render(ShaderHandler& shaderHandler, const glm::vec3& position, eFactionController owningFactionController, glm::vec3 rotation) const
{
	render(shaderHandler, position, false, rotation, owningFactionController);
}

void Model::render(ShaderHandler& shaderHandler, const Entity& entity) const
{
	render(shaderHandler, entity.getPosition(), entity.isSelected(), entity.getRotation());
}

void Model::render(ShaderHandler& shaderHandler, const Entity& entity, eFactionController owningFactionController) const
{
	switch (owningFactionController)
	{
	case eFactionController::Player:
		render(shaderHandler, entity.getPosition(), entity.isSelected(), entity.getRotation(), owningFactionController);
		break;
	case eFactionController::AI_1:
	case eFactionController::AI_2:
	case eFactionController::AI_3:
		render(shaderHandler, entity.getPosition(), false, entity.getRotation(), owningFactionController);
		break;
	default:
		assert(false);
	}
}
#endif // GAME