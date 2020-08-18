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
		{ 3, 1.0f, 3 }, eModelName::Unit, Globals::UNIT_AABB_SIZE);
	assert(unitModel);
	if (!unitModel)
	{
		std::cout << "Failed to load SpaceCraft model\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(unitModel->modelName)] = std::move(unitModel);

	std::unique_ptr<Model> headquartersModel = Model::create("portal.obj", true,
		glm::vec3(9.0f, 1.0f, 3.0f), eModelName::HQ, Globals::HQ_AABB_SIZE);
	assert(headquartersModel);
	if (!headquartersModel)
	{
		std::cout << "Failed to load portal model\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(headquartersModel->modelName)] = std::move(headquartersModel);

	std::unique_ptr<Model> mineralModel = Model::create("rocksOre.obj", true,
		glm::vec3(3.0f, 1.0f, 3.0f), eModelName::Mineral, Globals::MINERAL_AABB_SIZE);
	assert(mineralModel);
	if (!mineralModel)
	{
		std::cout << "Rocks Ore Model not found\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(mineralModel->modelName)] = std::move(mineralModel);

	std::unique_ptr<Model> waypointModel = Model::create("laserSabel.obj", true,
		glm::vec3(2.0f, 1.0f, 2.0f), eModelName::Waypoint, Globals::WAYPOINT_AABB_SIZE);
	assert(waypointModel);
	if (!waypointModel)
	{
		std::cout << "Failed to load laserSabel Model\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(waypointModel->modelName)] = std::move(waypointModel);

	std::unique_ptr<Model> workerModel = Model::create("robot.obj", false,
		{ 1.5f, 1.0f, 1.5f }, eModelName::Worker, Globals::WORKER_AABB_SIZE);
	assert(workerModel);
	if (!workerModel)
	{
		std::cout << "Failed to load: robot.obj\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(workerModel->modelName)] = std::move(workerModel);

	std::unique_ptr<Model> supplyDepotModel = Model::create("satelliteDish.obj", false,
		glm::vec3(3.0f, 1.0f, 3.0f), eModelName::SupplyDepot, Globals::SUPPLY_DEPOT_AABB_SIZE);
	assert(supplyDepotModel);
	if (!supplyDepotModel)
	{
		std::cout << "failed to load satelliteDishModel\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(supplyDepotModel->modelName)] = std::move(supplyDepotModel);

	std::unique_ptr<Model> barracksModel = Model::create("buildingOpen.obj", true,
		glm::vec3(3.0f, 1.0f, 3.0f), eModelName::Barracks, Globals::BARRACKS_AABB_SIZE);
	assert(barracksModel);
	if (!barracksModel)
	{
		std::cout << "Couldn't load buildingOpen.obj\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(barracksModel->modelName)] = std::move(barracksModel);
}