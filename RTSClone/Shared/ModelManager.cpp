#include "ModelManager.h"
#ifdef GAME
#include "EntityType.h"
#endif // GAME
#include <iostream>

namespace 
{
#ifdef GAME
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
#endif // GAME

	constexpr glm::vec3 UNIT_SCALE{ 0.35f, 0.35f, 0.35f };
	constexpr glm::vec3 HQ_SCALE{ 1.2f, 1.0f, 0.9f };
	constexpr glm::vec3 MINERAL_SCALE{ 0.6f, 0.6f, 0.6f };
	constexpr glm::vec3 WAYPOINT_SCALE{ 1.0f, 1.0f, 1.0f };
	constexpr glm::vec3 WORKER_SCALE{ 0.8f, 0.8f, 0.8f };
	constexpr glm::vec3 SUPPLY_DEPOT_SCALE{ 1.0f, 1.0f, 1.0f };
	constexpr glm::vec3 BARRACKS_SCALE{ 0.5f, 0.5f, 0.5f };
	constexpr glm::vec3 WORKER_MINERAL_SCALE = { 0.2f, 0.2f, 0.2f };
	constexpr glm::vec3 PROJECTILE_SCALE = { 0.75f, 0.75f, 0.75f };

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

#ifdef GAME
const Model& ModelManager::getModel(eEntityType entityType) const
{
	eModelName modelName = getModelName(entityType);
	assert(m_models[static_cast<int>(modelName)] &&
		m_models[static_cast<int>(modelName)]->modelName == modelName);

	return *m_models[static_cast<int>(modelName)];
}
#endif // GAME

ModelManager::ModelManager()
	: m_models(),
	m_loadedAllModels(true)
{
	loadModel("spaceCraft1.obj", false, UNIT_AABB_SIZE_FROM_CENTER, eModelName::Unit, UNIT_SCALE);
	loadModel("portal.obj", true, HQ_AABB_SIZE_FROM_CENTER, eModelName::HQ, HQ_SCALE);
	loadModel("rocksOre.obj", true, MINERAL_AABB_SIZE_FROM_CENTER, eModelName::Mineral, MINERAL_SCALE);
	loadModel("rocksOre.obj", true, WORKER_MINERAL_AABB_SIZE_FROM_CENTER, eModelName::WorkerMineral, WORKER_MINERAL_SCALE);
	loadModel("laserSabel.obj", false, WAYPOINT_AABB_SIZE_FROM_CENTER, eModelName::Waypoint, WAYPOINT_SCALE);
	loadModel("robot.obj", false, WORKER_AABB_SIZE_FROM_CENTER, eModelName::Worker, WORKER_SCALE);
	loadModel("laserSabel.obj", false, PROJECTILE_AABB_SIZE_FROM_CENTER, eModelName::Projectile, PROJECTILE_SCALE);
	loadModel("satelliteDish.obj", false, SUPPLY_DEPOT_AABB_SIZE_FROM_CENTER, eModelName::SupplyDepot, SUPPLY_DEPOT_SCALE);
	loadModel("buildingOpen.obj", true, BARRACKS_AABB_SIZE_FROM_CENTER, eModelName::Barracks, BARRACKS_SCALE);
}

void ModelManager::loadModel(const std::string& fileName, bool renderFromCenterPosition, const glm::vec3& AABBSizeFromCenter, 
	eModelName modelName, const glm::vec3& scale)
{
	std::unique_ptr<Model> model = Model::create(fileName, renderFromCenterPosition,
		AABBSizeFromCenter, modelName, scale);
	assert(model);
	if (!model)
	{
		std::cout << "Failed to load " << fileName << "\n";
		m_loadedAllModels = false;
	}

	m_models[static_cast<int>(model->modelName)] = std::move(model);
}