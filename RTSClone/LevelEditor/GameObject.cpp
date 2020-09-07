#include "GameObject.h"
#include "ModelManager.h"

GameObject::GameObject(eModelName modelName, const glm::vec3& startingPosition, const glm::vec3& startingScale)
	: modelName(modelName),
	position(startingPosition),
	scale(startingScale)
{}

void GameObject::render(ShaderHandler& shaderHandler) const
{
	ModelManager::getInstance().getModel(modelName).render(shaderHandler, *this);
}