#include "ModelManager.h"
#ifdef GAME
#include "EntityType.h"
#endif // GAME
#ifdef LEVEL_EDITOR
#include "Globals.h"
#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <functional>
#endif // LEVEL_EDITOR
#include <iostream>

const std::string TERRAIN_MODEL_NAME = "terrain.obj";
const std::string HQ_MODEL_NAME = "portal.obj";
const std::string MINERALS_MODEL_NAME = "rocksOre.obj";
const std::string UNIT_MODEL_NAME = "spaceCraft1.obj";
const std::string WORKER_MODEL_NAME = "robot.obj";
const std::string SUPPLY_DEPOT_MODEL_NAME = "satelliteDish.obj";
const std::string BARRACKS_MODEL_NAME = "buildingOpen.obj";
const std::string WAYPOINT_MODEL_NAME = "laserSabel.obj";
const std::string PROJECTILE_MODEL_NAME = "laserSabel.obj";
const std::string TRANSLATE_MODEL_NAME = "translate.obj";

namespace 
{
	constexpr glm::vec3 UNIT_SCALE{ 0.35f, 0.35f, 0.35f };
	constexpr glm::vec3 HQ_SCALE{ 1.2f, 1.2f, 0.9f };
	constexpr glm::vec3 MINERAL_SCALE{ 0.75f, 0.75f, 0.75f };
	constexpr glm::vec3 WAYPOINT_SCALE{ 1.0f, 1.0f, 1.0f };
	constexpr glm::vec3 WORKER_SCALE{ 1.0f, 1.0f, 1.0f };
	constexpr glm::vec3 SUPPLY_DEPOT_SCALE{ 1.0f, 1.0f, 1.0f };
	constexpr glm::vec3 BARRACKS_SCALE{ 0.5f, 0.5f, 0.5f };
	constexpr glm::vec3 WORKER_MINERAL_SCALE = { 0.2f, 0.2f, 0.2f };
	constexpr glm::vec3 PROJECTILE_SCALE = { 0.75f, 0.75f, 0.75f };

	constexpr glm::vec3 UNIT_AABB_SIZE_FROM_CENTER = { 2.9f, 1.0f, 2.9f };
	constexpr glm::vec3 HQ_AABB_SIZE_FROM_CENTER = { 9.0f, 1.0f, 3.0f };
	constexpr glm::vec3 MINERAL_AABB_SIZE_FROM_CENTER = { 3.0f, 1.0f, 3.0f };
	constexpr glm::vec3 WORKER_MINERAL_AABB_SIZE_FROM_CENTER = { 0.0f, 0.0f, 0.0f };
	constexpr glm::vec3 WAYPOINT_AABB_SIZE_FROM_CENTER = { 2.0f, 1.0f, 2.0f };
	constexpr glm::vec3 WORKER_AABB_SIZE_FROM_CENTER = { 1.5f, 1.0f, 1.5f };
	constexpr glm::vec3 PROJECTILE_AABB_SIZE_FROM_CENTER = { 1.0f, 1.0f, 1.0f };
	constexpr glm::vec3 SUPPLY_DEPOT_AABB_SIZE_FROM_CENTER = { 3.0f, 1.0f, 3.0f };
	constexpr glm::vec3 BARRACKS_AABB_SIZE_FROM_CENTER = { 3.0f, 1.0f, 3.0f };
	
	void loadModel(const std::string& fileName, bool renderFromCenterPosition, const glm::vec3& AABBSizeFromCenter,
		const glm::vec3& scale, std::vector<std::unique_ptr<Model>>& models, bool& loadedAllModels)
	{
		std::unique_ptr<Model> model = Model::create(fileName, renderFromCenterPosition,
			AABBSizeFromCenter, scale);
		assert(model);
		if (!model)
		{
			std::cout << "Failed to load " << fileName << "\n";
			loadedAllModels = false;
		}

		assert(std::find_if(models.cbegin(), models.cend(), [&fileName](const auto& model)
		{
			return model->modelName == fileName;
		}) == models.cend());

		models.push_back(std::move(model));
	}

	void loadSharedModels(std::vector<std::unique_ptr<Model>>& models, bool& loadedAllModels) 
	{
		loadModel("terrain.obj", false, { 0.0f, 0.0f, 0.0f }, { 2000.0f, 1.0f, 2000.0f },
			models, loadedAllModels);

		loadModel("buildingCorridorOpen.obj", true, { 5.0f, 5.0f, 5.0f }, { 1.0f, 1.0f, 1.0f }, models, loadedAllModels);

		loadModel("buildingCorridorOpenEnd.obj", true, { 5.0f, 5.0f, 5.0f }, { 1.0f, 1.0f, 1.0f }, models, loadedAllModels);

		loadModel("alienBones.obj", true, { 5.0f, 5.0f, 5.0f }, { 1.0f, 1.0f, 1.0f }, models, loadedAllModels);

		loadModel("rocks_SmallA.obj", false, { 5.0f, 5.0f, 5.0f }, { 10.0f, 10.0f, 10.0f }, models, loadedAllModels);

		loadModel("meteorFull.obj", false, { 5.0f, 50.0f, 5.0f }, { 1.0f, 1.0f, 1.0f },
			models, loadedAllModels);

		loadModel("meteorHalf.obj", false, { 5.0f, 1.0f, 5.0f }, { 1.0f, 1.0f, 1.0f }, models, loadedAllModels);

		loadModel("rocksTall.obj", true, { 5.0f, 1.0f, 5.0f }, { 1.0f, 1.0f, 1.0f },
			models, loadedAllModels);

		loadModel("portal.obj", true, HQ_AABB_SIZE_FROM_CENTER, HQ_SCALE,
			models, loadedAllModels);

		loadModel("rocksOre.obj", true, MINERAL_AABB_SIZE_FROM_CENTER, MINERAL_SCALE,
			models, loadedAllModels);
	}

#ifdef GAME
	std::vector<std::unique_ptr<Model>> loadGameModels(bool& loadedAllModels)
	{
		std::vector<std::unique_ptr<Model>> models;

		loadSharedModels(models, loadedAllModels);

		loadModel("spaceCraft1.obj", false, UNIT_AABB_SIZE_FROM_CENTER, UNIT_SCALE, 
			models, loadedAllModels);

		loadModel("robot.obj", false, WORKER_AABB_SIZE_FROM_CENTER, WORKER_SCALE,
			models, loadedAllModels);

		loadModel("laserSabel.obj", false, PROJECTILE_AABB_SIZE_FROM_CENTER, PROJECTILE_SCALE,
			models, loadedAllModels);

		loadModel("satelliteDish.obj", false, SUPPLY_DEPOT_AABB_SIZE_FROM_CENTER, SUPPLY_DEPOT_SCALE,
			models, loadedAllModels);

		loadModel("buildingOpen.obj", true, BARRACKS_AABB_SIZE_FROM_CENTER, BARRACKS_SCALE,
			models, loadedAllModels);

		return models;
	}
#endif // GAME

#ifdef LEVEL_EDITOR
	std::vector<std::unique_ptr<Model>> loadLevelEditorModels(bool& loadedAllModels)
	{
		std::vector<std::unique_ptr<Model>> models;
		loadSharedModels(models, loadedAllModels);

		loadModel("translate.obj", true, {0.0f, 0.0f, 0.0f}, { 15.0f, 15.0f, 15.0f }, models, loadedAllModels);

		return models;
	}

	std::vector<std::string> loadInModelNames()
	{
		std::vector<std::string> modelNames;

		std::filesystem::directory_iterator iter("../Shared/Models/");
		while (iter != std::filesystem::directory_iterator()) //Reach end
		{
			if (iter->path().extension() == ".obj") 
			{				
				modelNames.push_back(iter->path().filename().string());
			}

			++iter;
		}

		return modelNames;
	}

#endif // LEVEL_EDITOR
}

const Model& ModelManager::getModel(const std::string& modelName) const
{
	auto model = std::find_if(m_models.cbegin(), m_models.cend(), [&modelName](const auto& model)
	{
		return model->modelName == modelName;
	});
	assert(model != m_models.cend());

	return *(*model);
}

bool ModelManager::isAllModelsLoaded() const
{
	return m_loadedAllModels;
}

#ifdef LEVEL_EDITOR
bool ModelManager::isModelLoaded(const std::string& modelName) const
{
	auto model = std::find_if(m_models.cbegin(), m_models.cend(), [&modelName](const auto& model)
	{
		return model->modelName == modelName;
	});
	return model != m_models.cend();
}

const std::vector<std::string>& ModelManager::getModelNames() const
{
	return m_modelNames;
}

//ModelManager
ModelManager::ModelManager()
	: m_loadedAllModels(true),
	m_models(loadLevelEditorModels(m_loadedAllModels)),
	m_modelNames(loadInModelNames())
{}
#endif // LEVEL_EDITOR

#ifdef GAME
ModelManager::ModelManager()
	: m_loadedAllModels(true),
	m_models(loadGameModels(m_loadedAllModels))
{}
#endif // GAME
