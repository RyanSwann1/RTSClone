#pragma once

#include "Model.h"
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

class ModelManager : private NonMovable, private NonCopyable
{
public:
	static ModelManager& getInstance() 
	{
		static ModelManager instance;
		return instance;
	}

	const Model& getModel(const std::string& modelName) const;
	bool isAllModelsLoaded() const;

#ifdef LEVEL_EDITOR
	bool isModelLoaded(const std::string& modelName) const;
	const std::vector<std::string>& getModelNames() const;
#endif // LEVEL_EDITOR

private:
	ModelManager();
	bool m_loadedAllModels;
	const std::vector<std::unique_ptr<Model>> m_models;
	const std::vector<std::string> m_modelNames;
};