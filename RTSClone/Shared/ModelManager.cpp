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

	void loadModel(const std::string& fileName, bool renderFromCenterPosition, const glm::vec3& AABBSizeFromCenter,
		eModelName modelName, const glm::vec3& scale, std::array<std::unique_ptr<Model>, static_cast<size_t>(eModelName::Max) + 1>& models,
		bool& loadedAllModels)
	{
		std::unique_ptr<Model> model = Model::create(fileName, renderFromCenterPosition,
			AABBSizeFromCenter, modelName, scale);
		assert(model);
		if (!model)
		{
			std::cout << "Failed to load " << fileName << "\n";
			loadedAllModels = false;
		}

		assert(!models[static_cast<int>(model->modelName)]);
		models[static_cast<int>(model->modelName)] = std::move(model);
	}

	std::array<std::unique_ptr<Model>, static_cast<size_t>(eModelName::Max) + 1> loadModels(bool& loadedAllModels)
	{
		std::array<std::unique_ptr<Model>, static_cast<size_t>(eModelName::Max) + 1> models;
		loadModel("terrain.obj", false, { 0.0f, 0.0f, 0.0f }, eModelName::Terrain, { 500.0f, 1.0f, 500.0f },
			models, loadedAllModels);

		loadModel("meteorFull.obj", false, { 5.0f, 1.0f, 5.0f }, eModelName::Meteor, { 1.0f, 1.0f, 1.0f },
			models, loadedAllModels);

		loadModel("rocksTall.obj", true, { 5.0f, 1.0f, 5.0f }, eModelName::RocksTall, { 1.0f, 1.0f, 1.0f },
			models, loadedAllModels);

		loadModel("spaceCraft1.obj", false, UNIT_AABB_SIZE_FROM_CENTER, eModelName::Unit, UNIT_SCALE,
			models, loadedAllModels);
		
		loadModel("portal.obj", true, HQ_AABB_SIZE_FROM_CENTER, eModelName::HQ, HQ_SCALE,
			models, loadedAllModels);
		
		loadModel("rocksOre.obj", true, MINERAL_AABB_SIZE_FROM_CENTER, eModelName::Mineral, MINERAL_SCALE,
			models, loadedAllModels);
		
		loadModel("rocksOre.obj", true, WORKER_MINERAL_AABB_SIZE_FROM_CENTER, eModelName::WorkerMineral, WORKER_MINERAL_SCALE,
			models, loadedAllModels);
		
		loadModel("laserSabel.obj", false, WAYPOINT_AABB_SIZE_FROM_CENTER, eModelName::Waypoint, WAYPOINT_SCALE,
			models, loadedAllModels);
		
		loadModel("robot.obj", false, WORKER_AABB_SIZE_FROM_CENTER, eModelName::Worker, WORKER_SCALE,
			models, loadedAllModels);
		
		loadModel("laserSabel.obj", false, PROJECTILE_AABB_SIZE_FROM_CENTER, eModelName::Projectile, PROJECTILE_SCALE,
			models, loadedAllModels);
		
		loadModel("satelliteDish.obj", false, SUPPLY_DEPOT_AABB_SIZE_FROM_CENTER, eModelName::SupplyDepot, SUPPLY_DEPOT_SCALE,
			models, loadedAllModels);
		
		loadModel("buildingOpen.obj", true, BARRACKS_AABB_SIZE_FROM_CENTER, eModelName::Barracks, BARRACKS_SCALE,
			models, loadedAllModels);
		

		assert(std::find(models.cbegin(), models.cend(), nullptr) == models.cend());
		return models;
	}

	std::unordered_map<std::string, eModelName> getModelNameConversions()
	{
		std::unordered_map<std::string, eModelName> modelNameConversions;

		modelNameConversions.emplace("Terrain", eModelName::Terrain);
		modelNameConversions.emplace("Meteor", eModelName::Meteor);
		modelNameConversions.emplace("RocksTall", eModelName::RocksTall);
		modelNameConversions.emplace("Unit", eModelName::Unit);
		modelNameConversions.emplace("HQ", eModelName::HQ);
		modelNameConversions.emplace("Mineral", eModelName::Mineral);
		modelNameConversions.emplace("WorkerMineral", eModelName::WorkerMineral);
		modelNameConversions.emplace("Waypoint", eModelName::Waypoint);
		modelNameConversions.emplace("Worker", eModelName::Worker);
		modelNameConversions.emplace("Projectile", eModelName::Projectile);
		modelNameConversions.emplace("SupplyDepot", eModelName::SupplyDepot);
		modelNameConversions.emplace("Barracks", eModelName::Barracks);
	
		return modelNameConversions;
	}
}

bool ModelManager::isAllModelsLoaded() const
{
	return m_loadedAllModels;
}

#ifdef LEVEL_EDITOR
const std::array<std::string, static_cast<size_t>(eModelName::Max) + 1> ModelManager::getModelNames() const
{
	return m_modelNames;
}

eModelName ModelManager::getModelName(const std::string& modelName) const
{
	auto iter = m_modelNameConversions.find(modelName);
	assert(iter != m_modelNameConversions.cend());

	return iter->second;
}
#endif // LEVEL_EDITOR

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
	: m_loadedAllModels(true),
	m_models(loadModels(m_loadedAllModels)),
#ifdef LEVEL_EDITOR
	m_modelNameConversions(getModelNameConversions())
#endif // LEVEL_EDITOR
{}