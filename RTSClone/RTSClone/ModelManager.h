#pragma once

#include "Model.h"
#include <memory>
#include <array>

class ModelManager : private NonMovable, private NonCopyable
{
public:
	static std::unique_ptr<ModelManager> create();

	const Model& getModel(eModelName modelName) const;

private:
	ModelManager(std::array<std::unique_ptr<Model>, static_cast<size_t>(eModelName::Max) + 1>&& models);
	
	std::array<std::unique_ptr<Model>, static_cast<size_t>(eModelName::Max) + 1> m_models;
};