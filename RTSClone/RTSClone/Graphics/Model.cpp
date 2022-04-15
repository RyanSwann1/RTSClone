#include "Graphics/Model.h"
#include "Graphics/ShaderHandler.h"
#include "Graphics/ModelLoader.h"
#include "Core/Globals.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#ifdef GAME
#include "Entities/Entity.h"
#include "Scene/SceneryGameObject.h"
#else
#include "../LevelEditor/Scene/GameObject.h"
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

void Model::attachMeshesToVAO() const
{
	for (const auto& mesh : meshes)
	{
		mesh.attachToVAO();
	}
}

void Model::setModelMatrix(ShaderHandler& shaderHandler, glm::vec3 position, const glm::vec3& rotation) const
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

void Model::render(ShaderHandler& shaderHandler, const glm::vec3& position, const glm::vec3& additionalColour, float opacity,
	glm::vec3 rotation) const
{
	setModelMatrix(shaderHandler, position, rotation);

	for (const auto& mesh : meshes)
	{
		mesh.render(shaderHandler, additionalColour, opacity);
	}
}

void Model::render(ShaderHandler& shaderHandler, const glm::vec3& position, glm::vec3 rotation, bool highlight) const
{	
	setModelMatrix(shaderHandler, position, rotation);

	for (const auto& mesh : meshes)
	{
		mesh.render(shaderHandler, highlight);
	}
}

#ifdef GAME
void Model::render(ShaderHandler& shaderHandler, eFactionController owningFactionController, const glm::vec3& position, 
	glm::vec3 rotation, bool highlight) const
{
	setModelMatrix(shaderHandler, position, rotation);

	for (const auto& mesh : meshes)
	{
		mesh.render(shaderHandler, owningFactionController, highlight);
	}
}

void Model::render(ShaderHandler& shaderHandler, const SceneryGameObject& gameObject) const
{
	setModelMatrix(shaderHandler, gameObject);

	for(const auto& mesh : meshes)
	{
		mesh.render(shaderHandler);
	}
}

void Model::setModelMatrix(ShaderHandler& shaderHandler, const SceneryGameObject& gameObject) const
{
	glm::vec3 gameObjectPosition = gameObject.position;
	glm::mat4 model = glm::mat4(1.0f);
	if (renderFromCentrePosition)
	{
		gameObjectPosition.x += AABBSizeFromCenter.x;
		gameObjectPosition.z -= AABBSizeFromCenter.z;

		model = glm::translate(model, gameObjectPosition);
		model = glm::scale(model, gameObject.scale);
		model = glm::translate(model, glm::vec3(-AABBSizeFromCenter.x, 0.0f, AABBSizeFromCenter.z));
		model = glm::rotate(model, glm::radians(gameObject.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(AABBSizeFromCenter.x, 0.0f, -AABBSizeFromCenter.z));
	}
	else
	{
		model = glm::translate(model, gameObjectPosition);
		model = glm::scale(model, gameObject.scale);
		model = glm::rotate(model, glm::radians(gameObject.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", model);
}
#else
void Model::render(ShaderHandler& shaderHandler, const GameObject& gameObject, bool highlight /*= false*/) const
{
	//setModelMatrix(shaderHandler, gameObject);

	for (const auto& mesh : meshes)
	{
		mesh.render(shaderHandler, highlight);
	}
}


#endif // GAME
#ifdef LEVEL_EDITOR
void Model::setModelMatrix(ShaderHandler& shaderHandler, const GameObject& gameObject) const
{
	glm::vec3 gameObjectPosition = gameObject.position;
	glm::vec3 gameObjectScale = scale;
	if (gameObject.useLocalScale)
	{
		gameObjectScale = gameObject.scale;
	}
	glm::mat4 model = glm::mat4(1.0f);
	if (renderFromCentrePosition)
	{
		gameObjectPosition.x += AABBSizeFromCenter.x;
		gameObjectPosition.z -= AABBSizeFromCenter.z;

		model = glm::translate(model, gameObjectPosition);
		model = glm::scale(model, gameObjectScale);
		model = glm::translate(model, glm::vec3(-AABBSizeFromCenter.x, 0.0f, AABBSizeFromCenter.z));
		model = glm::rotate(model, glm::radians(gameObject.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(AABBSizeFromCenter.x, 0.0f, -AABBSizeFromCenter.z));
	}
	else
	{
		model = glm::translate(model, gameObjectPosition);
		model = glm::scale(model, gameObjectScale);
		model = glm::rotate(model, glm::radians(gameObject.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", model);
}
#endif // GAME