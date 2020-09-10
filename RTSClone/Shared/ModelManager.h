#pragma once

#include "Model.h"
#include <memory>
#include <array>
#ifdef LEVEL_EDITOR
#include <unordered_map>
#endif // LEVEL_EDITOR

#ifdef GAME
enum class eEntityType;
#endif // GAME
class ModelManager : private NonMovable, private NonCopyable
{
public:
	static ModelManager& getInstance() 
	{
		static ModelManager instance;
		return instance;
	}

	bool isAllModelsLoaded() const;
#ifdef LEVEL_EDITOR
	const std::array<std::string, static_cast<size_t>(eModelName::Max) + 1> getModelNames() const;
	eModelName getModelName(const std::string& modelName) const;
#endif // LEVEL_EDITOR
	
	const Model& getModel(eModelName modelName) const;
#ifdef GAME
	const Model& getModel(eEntityType entityType) const;
#endif // GAME

private:
	ModelManager();
	bool m_loadedAllModels;
	const std::array<std::unique_ptr<Model>, static_cast<size_t>(eModelName::Max) + 1> m_models;
#ifdef LEVEL_EDITOR
	const std::unordered_map<std::string, eModelName> m_modelNameConversions;
	const std::array<std::string, static_cast<size_t>(eModelName::Max) + 1> m_modelNames
	{
		"Terrain",
		"Meteor",
		"RocksTall",
		"Unit",
		"HQ",
		"Mineral",
		"WorkerMineral",
		"Waypoint",
		"Worker",
		"Projectile",
		"SupplyDepot",
		"Barracks"
	};
#endif // LEVEL_EDITOR
};