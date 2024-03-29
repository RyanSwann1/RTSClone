#pragma once

#include "Graphics/Model.h"
#ifdef GAME
#include "Entities/EntityType.h"
#endif // GAME

#include <memory>
#include <array>

//Commonly Refferred to game models
extern const std::string TERRAIN_MODEL_NAME;
extern const std::string HQ_MODEL_NAME;
extern const std::string MINERALS_MODEL_NAME;
extern const std::string UNIT_MODEL_NAME;
extern const std::string WORKER_MODEL_NAME;
extern const std::string SUPPLY_DEPOT_MODEL_NAME;
extern const std::string BARRACKS_MODEL_NAME;
extern const std::string WAYPOINT_MODEL_NAME;
extern const std::string PROJECTILE_MODEL_NAME;
extern const std::string TRANSLATE_MODEL_NAME;
extern const std::string TURRET_MODEL_NAME;
extern const std::string LABORATORY_MODEL_NAME;
#ifdef GAME
extern const std::array<std::string, static_cast<size_t>(eEntityType::Max) + 1> MODEL_NAMES;
#endif // GAME

#ifdef GAME
enum class eEntityType;
class AABB;
#endif // GAME
class ModelManager
{
public:
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;
	ModelManager(ModelManager&&) = delete;
	ModelManager& operator=(ModelManager&&) = delete;
	static ModelManager& getInstance() 
	{
		static ModelManager instance;
		return instance;
	}

#ifdef GAME
	const Model& getModel(eEntityType entityType) const;
	AABB getModelAABB(const glm::vec3& position, eEntityType entityType) const;
#endif // GAME

	const Model& getModel(const std::string& modelName) const;
	bool isAllModelsLoaded() const;

#ifdef LEVEL_EDITOR
	Model& getModel(const std::string& modelName);
	bool isModelLoaded(const std::string& modelName) const;
	const std::vector<std::string>& getModelNames() const;
#endif // LEVEL_EDITOR

private:
	ModelManager();
	bool m_loadedAllModels;
	const std::vector<std::unique_ptr<Model>> m_models;
	const std::vector<std::string> m_modelNames;
};