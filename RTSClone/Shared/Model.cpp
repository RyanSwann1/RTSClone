#include "Model.h"
#include "ShaderHandler.h"
#include "ModelLoader.h"
#include "Entity.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

Model::Model(bool renderFromCentrePosition, const glm::vec3& AABBSizeFromCenter, eModelName modelName, 
	const glm::vec3& scale)
	: modelName(modelName),
	renderFromCentrePosition(renderFromCentrePosition),
	AABBSizeFromCenter(AABBSizeFromCenter),
	scale(scale),
	meshes(),
	textures()
{}

void Model::render(ShaderHandler & shaderHandler, glm::vec3 entityPosition, bool entitySelected) const
{
	if (renderFromCentrePosition)
	{
		entityPosition.x += AABBSizeFromCenter.x;
		entityPosition.z -= AABBSizeFromCenter.z;
	}

	glm::mat4 model = glm::translate(glm::mat4(1.0f), entityPosition);
	model = glm::scale(model, scale);
	shaderHandler.setUniformMat4f(eShaderType::Default, "uModel", model);

	for (const auto& mesh : meshes)
	{
		mesh.render(shaderHandler, entitySelected);
	}
}

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

bool Model::isCollidable() const
{
	switch (modelName)
	{
		case eModelName::Meteor:
		case eModelName::BuildingCorridorOpen:
		case eModelName::BuildingCorridorOpenEnd:
		case eModelName::AlienBones:
		case eModelName::RocksTall:
		case eModelName::HQ:
		case eModelName::WorkerMineral:
		case eModelName::Worker:
		case eModelName::SupplyDepot:
		case eModelName::Barracks:
			return true;
		case eModelName::Terrain:
		case eModelName::Unit:
		case eModelName::RocksSmallA:
		case eModelName::Waypoint:
		case eModelName::TranslateObject:
			return false;
		default:
			assert(false);
			return false;
	}
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
	render(shaderHandler, entity.getPosition(), entity.isSelected());
}