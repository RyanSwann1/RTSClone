#pragma once

#include "Model.h"
#include <memory>
#include <array>

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
	const Model& getModel(eModelName modelName) const;
#ifdef GAME
	const Model& getModel(eEntityType entityType) const;
#endif // GAME

private:
	ModelManager();
	std::array<std::unique_ptr<Model>, static_cast<size_t>(eModelName::Max) + 1> m_models;
	bool m_loadedAllModels;

	void loadModel(const std::string& fileName, bool renderFromCenterPosition, const glm::vec3& AABBSizeFromCenter,
		eModelName modelName, const glm::vec3& scale);
};