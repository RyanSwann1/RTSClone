#include "ModelManager.h"
#include "Globals.h"
#include "EntityType.h"
#include <iostream>

namespace 
{
	eModelName getModelName(eEntityType entityType)
	{
		switch (entityType)
		{
		case eEntityType::Unit:
			return eModelName::Unit;
		case eEntityType::Worker:
			return eModelName::Worker;
		case eEntityType::HQ:
			return eModelName::HQ;
		case eEntityType::SupplyDepot:
			return eModelName::SupplyDepot;
		case eEntityType::Barracks:
			return eModelName::Barracks;
		case eEntityType::Mineral:
			return eModelName::Mineral;
		case eEntityType::Projectile:
			return eModelName::Projectile;
		default:
			assert(false);
		}
	}

	constexpr glm::vec3 UNIT_SCALE{ 0.35f, 0.35f, 0.35f };
	constexpr glm::vec3 HQ_SCALE{ 1.2f, 1.0f, 0.9f };
	constexpr glm::vec3 MINERAL_SCALE{ 0.6f, 0.6f, 0.6f };
	constexpr glm::vec3 WAYPOINT_SCALE{ 1.0f, 1.0f, 1.0f };
	constexpr glm::vec3 WORKER_SCALE{ 0.8f, 0.8f, 0.8f };
	constexpr glm::vec3 SUPPLY_DEPOT_SCALE{ 1.0f, 1.0f, 1.0f };
	constexpr glm::vec3 BARRACKS_SCALE{ 0.5f, 0.5f, 0.5f };

	constexpr glm::vec3 UNIT_AABB_SIZE_FROM_CENTER = { 3.0f, 1.0f, 3.0f };
	constexpr glm::vec3 HQ_AABB_SIZE_FROM_CENTER = { 9.0f, 1.0f, 3.0f };
	constexpr glm::vec3 MINERAL_AABB_SIZE_FROM_CENTER = { 3.0f, 1.0f, 3.0f };
	constexpr glm::vec3 WORKER_MINERAL_AABB_SIZE_FROM_CENTER = { 0.0f, 0.0f, 0.0f };
	constexpr glm::vec3 WAYPOINT_AABB_SIZE_FROM_CENTER = { 2.0f, 1.0f, 2.0f };
	constexpr glm::vec3 WORKER_AABB_SIZE_FROM_CENTER = { 1.5f, 1.0f, 1.5f };
	constexpr glm::vec3 PROJECTILE_AABB_SIZE_FROM_CENTER = { 1.0f, 1.0f, 1.0f };
	constexpr glm::vec3 SUPPLY_DEPOT_AABB_SIZE_FROM_CENTER = { 3.0f, 1.0f, 3.0f };
	constexpr glm::vec3 BARRACKS_AABB_SIZE_FROM_CENTER = { 3.0f, 1.0f, 3.0f };
}

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

const Model& ModelManager::getModel(eEntityType entityType) const
{
	eModelName modelName = getModelName(entityType);
	assert(m_models[static_cast<int>(modelName)] &&
		m_models[static_cast<int>(modelName)]->modelName == modelName);

	return *m_models[static_cast<int>(modelName)];
}

ModelManager::ModelManager()
	: m_models(),
	m_loadedAllModels(true)
{
	std::unique_ptr<Model> unitModel = Model::create("spaceCraft1.obj", false,
		UNIT_AABB_SIZE_FROM_CENTER, eModelName::Unit, UNIT_SCALE);
	assert(unitModel);
	if (!unitModel)
	{
		std::cout << "Failed to load SpaceCraft model\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(unitModel->modelName)] = std::move(unitModel);

	std::unique_ptr<Model> headquartersModel = Model::create("portal.obj", true,
		HQ_AABB_SIZE_FROM_CENTER, eModelName::HQ, HQ_SCALE);
	assert(headquartersModel);
	if (!headquartersModel)
	{
		std::cout << "Failed to load portal model\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(headquartersModel->modelName)] = std::move(headquartersModel);

	std::unique_ptr<Model> mineralModel = Model::create("rocksOre.obj", true,
		MINERAL_AABB_SIZE_FROM_CENTER, eModelName::Mineral, MINERAL_SCALE);
	assert(mineralModel);
	if (!mineralModel)
	{
		std::cout << "Rocks Ore Model not found\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(mineralModel->modelName)] = std::move(mineralModel);

	std::unique_ptr<Model> workerMineralModel = Model::create("rocksOre.obj", true,
		WORKER_MINERAL_AABB_SIZE_FROM_CENTER, eModelName::WorkerMineral , { 0.2f, 0.2f, 0.2f });
	assert(workerMineralModel);
	if (!workerMineralModel)
	{
		std::cout << "Rocks Ore Model not found\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(workerMineralModel->modelName)] = std::move(workerMineralModel);

	std::unique_ptr<Model> waypointModel = Model::create("laserSabel.obj", false,
		WAYPOINT_AABB_SIZE_FROM_CENTER, eModelName::Waypoint, WAYPOINT_SCALE);
	assert(waypointModel);
	if (!waypointModel)
	{
		std::cout << "Failed to load laserSabel Model\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(waypointModel->modelName)] = std::move(waypointModel);

	std::unique_ptr<Model> workerModel = Model::create("robot.obj", false,
		WORKER_AABB_SIZE_FROM_CENTER, eModelName::Worker, WORKER_SCALE);
	assert(workerModel);
	if (!workerModel)
	{
		std::cout << "Failed to load: robot.obj\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(workerModel->modelName)] = std::move(workerModel);

	std::unique_ptr<Model> projectileModel = Model::create("laserSabel.obj", false,
		PROJECTILE_AABB_SIZE_FROM_CENTER, eModelName::Projectile, { 0.75f, 0.75f, 0.75f });
	assert(projectileModel);
	if (!projectileModel)
	{
		std::cout << "Couldn't load projectile model\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(eModelName::Projectile)] = std::move(projectileModel);

	std::unique_ptr<Model> supplyDepotModel = Model::create("satelliteDish.obj", false,
		SUPPLY_DEPOT_AABB_SIZE_FROM_CENTER, eModelName::SupplyDepot, SUPPLY_DEPOT_SCALE);
	assert(supplyDepotModel);
	if (!supplyDepotModel)
	{
		std::cout << "failed to load satelliteDishModel\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(supplyDepotModel->modelName)] = std::move(supplyDepotModel);

	std::unique_ptr<Model> barracksModel = Model::create("buildingOpen.obj", true,
		BARRACKS_AABB_SIZE_FROM_CENTER, eModelName::Barracks, BARRACKS_SCALE);
	assert(barracksModel);
	if (!barracksModel)
	{
		std::cout << "Couldn't load buildingOpen.obj\n";
		m_loadedAllModels = false;
	}
	m_models[static_cast<int>(barracksModel->modelName)] = std::move(barracksModel);
}