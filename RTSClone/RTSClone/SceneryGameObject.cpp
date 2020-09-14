#include "SceneryGameObject.h"
#include "GameMessenger.h"
#include "GameMessages.h"
#include "ModelManager.h"

SceneryGameObject::SceneryGameObject(eModelName modelName, const glm::vec3& position)
	: modelName(modelName),
	position(position)
{}

SceneryGameObject::SceneryGameObject(SceneryGameObject&& orig) noexcept
	: modelName(orig.modelName),
	position(orig.position)
{}

SceneryGameObject& SceneryGameObject::operator=(SceneryGameObject&& orig) noexcept
{
	modelName = orig.modelName;
	position = orig.position;

	return *this;
}

void SceneryGameObject::render(ShaderHandler& shaderHandler) const
{
	ModelManager::getInstance().getModel(modelName).render(shaderHandler, position);
}