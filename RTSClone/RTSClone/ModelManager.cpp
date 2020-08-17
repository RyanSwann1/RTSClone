#include "ModelManager.h"
#include "Globals.h"
#include <iostream>

bool ModelManager::isAllModelsLoaded() const
{
	return m_loadedAllModels;
}

const Model& ModelManager::getModel(eModelName modelName) const
{
	assert(m_models[static_cast<int>(modelName)] &&
		m_models[static_cast<int>(modelName)]->modelName == modelName);

	return *m_models[static_cast<int>(modelName)];
}

ModelManager::ModelManager()
	: m_models(),
	m_loadedAllModels(true)
{
	std::unique_ptr<Model> unitModel = Model::create("spaceCraft1.obj", false,
		{ 3, 1.0f, 3 }, eModelName::Unit, { 0.35f, 0.35f, 0.35f });
	assert(unitModel);
	if (!unitModel)
	{
		std::cout << "Failed to load SpaceCraft model\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(unitModel->modelName)] = std::move(unitModel);

	std::unique_ptr<Model> headquartersModel = Model::create("portal.obj", true,
		glm::vec3(9.0f, 1.0f, 3.0f), eModelName::HQ, { 1.2f, 1.0f, 0.9f });
	assert(headquartersModel);
	if (!headquartersModel)
	{
		std::cout << "Failed to load portal model\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(headquartersModel->modelName)] = std::move(headquartersModel);

	std::unique_ptr<Model> mineralModel = Model::create("rocksOre.obj", true,
		glm::vec3(3.0f, 1.0f, 3.0f), eModelName::Mineral, { 0.6f, 0.6f, 0.6f });
	assert(mineralModel);
	if (!mineralModel)
	{
		std::cout << "Rocks Ore Model not found\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(mineralModel->modelName)] = std::move(mineralModel);

	std::unique_ptr<Model> waypointModel = Model::create("laserSabel.obj", true,
		glm::vec3(2.0f, 1.0f, 2.0f), eModelName::Waypoint, { 1.0f, 1.0f, 1.0f });
	assert(waypointModel);
	if (!waypointModel)
	{
		std::cout << "Failed to load laserSabel Model\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(waypointModel->modelName)] = std::move(waypointModel);

	std::unique_ptr<Model> harvesterModel = Model::create("robot.obj", false,
		{ 1.5f, 1.0f, 1.5f }, eModelName::Worker, { 0.8f, 0.8f, 0.8f });
	assert(harvesterModel);
	if (!harvesterModel)
	{
		std::cout << "Failed to load: robot.obj\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(harvesterModel->modelName)] = std::move(harvesterModel);

	std::unique_ptr<Model> satelliteDishModel = Model::create("satelliteDish.obj", false,
		glm::vec3(3.0f, 1.0f, 3.0f), eModelName::SupplyDepot, glm::vec3(1.0f, 1.0f, 1.0f));
	assert(satelliteDishModel);
	if (!satelliteDishModel)
	{
		std::cout << "failed to load satelliteDishModel\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(satelliteDishModel->modelName)] = std::move(satelliteDishModel);

	std::unique_ptr<Model> barracksModel = Model::create("buildingOpen.obj", true,
		glm::vec3(3.0f, 1.0f, 3.0f), eModelName::Barracks, { 0.5f, 0.5f, 0.5f });
	assert(barracksModel);
	if (!barracksModel)
	{
		std::cout << "Couldn't load buildingOpen.obj\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(barracksModel->modelName)] = std::move(barracksModel);
}