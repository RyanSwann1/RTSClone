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

	const Model& getModel(eModelName modelName) const;
	bool isAllModelsLoaded() const;

#ifdef LEVEL_EDITOR
	static const size_t TOTAL_MODEL_NAMES = 3;
	const std::array<std::string, TOTAL_MODEL_NAMES> getModelNames() const;
	eModelName getModelName(const std::string& modelName) const;
#endif // LEVEL_EDITOR

#ifdef GAME
	const Model& getModel(eEntityType entityType) const;
#endif // GAME

private:
	ModelManager();
	bool m_loadedAllModels;
	const std::array<std::unique_ptr<Model>, static_cast<size_t>(eModelName::Max) + 1> m_models;

#ifdef LEVEL_EDITOR	
	const std::unordered_map<std::string, eModelName> m_modelNameConversions
	{
		{"Meteor", eModelName::Meteor},
		{"RocksTall", eModelName::RocksTall},
		{"MeteorHalf", eModelName::MeteorHalf}
	};
	const std::array<std::string, TOTAL_MODEL_NAMES> m_modelNames
	{
		"Meteor",
		"MeteorHalf",
		"RocksTall"
	};
#endif // LEVEL_EDITOR
};