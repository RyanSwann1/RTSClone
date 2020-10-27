#include "Model.h"
#include "ShaderHandler.h"
#include "ModelLoader.h"
#include "Entity.h"
#include "Globals.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

Model::Model(bool renderFromCentrePosition, const glm::vec3& AABBSizeFromCenter, const glm::vec3& scale,
	const std::string& fileName, std::vector<Mesh>&& meshes)
	: modelName(fileName),
	renderFromCentrePosition(renderFromCentrePosition),
	AABBSizeFromCenter(AABBSizeFromCenter),
	scale(scale),
	meshes(std::move(meshes)),
	textures()
{}

void Model::render(ShaderHandler & shaderHandler, glm::vec3 entityPosition, bool entitySelected, glm::vec3 rotation) const
{
	glm::mat4 model = glm::mat4(1.0f);
	if (renderFromCentrePosition)
	{
		entityPosition.x += AABBSizeFromCenter.x;
		entityPosition.z -= AABBSizeFromCenter.z;

		model = glm::translate(model, entityPosition);
		model = glm::scale(model, scale);
		model = glm::translate(model, glm::vec3(-AABBSizeFromCenter.x, 0.0f, AABBSizeFromCenter.z));
		model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(AABBSizeFromCenter.x, 0.0f, -AABBSizeFromCenter.z));
	}
	else
	{
		model = glm::translate(model, entityPosition);
		model = glm::scale(model, scale);
		model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	
	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", model);

	for (const auto& mesh : meshes)
	{
		mesh.render(shaderHandler, entitySelected);
	}
}

std::unique_ptr<Model> Model::create(const std::string & fileName, bool renderFromCentrePosition, 
	const glm::vec3& AABBSizeFromCenter, const glm::vec3& scale)
{
	std::vector<Mesh> meshes;
	if (!ModelLoader::loadModel(fileName, meshes))
	{
		return std::unique_ptr<Model>();
	}

	Model* model = new Model(renderFromCentrePosition, AABBSizeFromCenter, scale, fileName, std::move(meshes));
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
	render(shaderHandler, position, false);
}

void Model::render(ShaderHandler& shaderHandler, const Entity& entity) const
{
	render(shaderHandler, entity.getPosition(), entity.isSelected(), entity.getRotation());
}