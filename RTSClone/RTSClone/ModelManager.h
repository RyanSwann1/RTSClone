#pragma once

#include "Model.h"
#include <memory>
#include <array>

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

private:
	ModelManager();
	std::array<std::unique_ptr<Model>, static_cast<size_t>(eModelName::Max) + 1> m_models;
	bool m_loadedAllModels;
};