#include "ModelManager.h"
#include <iostream>

std::unique_ptr<ModelManager> ModelManager::create()
{
	std::array<std::unique_ptr<Model>, static_cast<size_t>(eModelName::Max) + 1> models;

	std::unique_ptr<Model> unitModel = Model::create("spaceCraft1.obj", false, glm::vec3(5.0f, 0.25f, 5.0f), eModelName::Unit);
	assert(unitModel);
	if (!unitModel)
	{
		std::cout << "Failed to load SpaceCraft model\n";
		return std::unique_ptr<ModelManager>();
	}
	models[static_cast<int>(unitModel->modelName)] = std::move(unitModel);

	std::unique_ptr<Model> headquartersModel = Model::create("portal.obj", true, glm::vec3(5.0f, 0.25f, 3.0f), eModelName::HQ);
	assert(headquartersModel);
	if (!headquartersModel)
	{
		std::cout << "Failed to load portal model\n";
		return std::unique_ptr<ModelManager>();
	}
	models[static_cast<int>(headquartersModel->modelName)] = std::move(headquartersModel);

	std::unique_ptr<Model> mineralModel = Model::create("rocksOre.obj", true, glm::vec3(5.0f, 0.25f, 5.0f), eModelName::Mineral);
	assert(mineralModel);
	if (!mineralModel)
	{
		std::cout << "Rocks Ore Model not found\n";
		return std::unique_ptr<ModelManager>();
	}
	models[static_cast<int>(mineralModel->modelName)] = std::move(mineralModel);

	std::unique_ptr<Model> waypointModel = Model::create("laserSabel.obj", true, glm::vec3(2.0f, 1.0f, 2.0f), eModelName::Waypoint);
	assert(waypointModel);
	if (!waypointModel)
	{
		std::cout << "Failed to load laserSabel Model\n";
		return std::unique_ptr<ModelManager>();
	}
	models[static_cast<int>(waypointModel->modelName)] = std::move(waypointModel);

	std::unique_ptr<Model> harvesterModel = Model::create("robot.obj", false, { 2.0f, 1.0f, 2.0 }, eModelName::Harvester);
	assert(harvesterModel);
	if (!harvesterModel)
	{
		std::cout << "Failed to load: robot.obj\n";
		return std::unique_ptr<ModelManager>();
	}
	models[static_cast<int>(harvesterModel->modelName)] = std::move(harvesterModel);
	
	return std::unique_ptr<ModelManager>(new ModelManager(std::move(models)));
}

const Model& ModelManager::getModel(eModelName modelName) const
{
	assert(m_models[static_cast<int>(modelName)] &&
		m_models[static_cast<int>(modelName)]->modelName == modelName);

	return *m_models[static_cast<int>(modelName)];
}

ModelManager::ModelManager(std::array<std::unique_ptr<Model>, static_cast<size_t>(eModelName::Max) + 1>&& models)
	: m_models(std::move(models))
{}