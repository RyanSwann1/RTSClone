#include "GameObject.h"
#include "ModelManager.h"

GameObject::GameObject(eModelName modelName, const glm::vec3& startingPosition)
	: modelName(modelName),
	position(startingPosition),
	AABB(startingPosition, ModelManager::getInstance().getModel(modelName))
{}

void GameObject::render(ShaderHandler& shaderHandler) const
{
	ModelManager::getInstance().getModel(modelName).render(shaderHandler, *this);
}

void GameObject::renderAABB(ShaderHandler& shaderHandler)
{
	AABB.render(shaderHandler);
}