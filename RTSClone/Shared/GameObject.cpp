#include "GameObject.h"
#include "ModelManager.h"

#ifdef LEVEL_EDITOR
GameObject::GameObject()
	: modelName(),
	position(),
	AABB(),
	selected(false)
{}
#endif // LEVEL_EDITOR

GameObject::GameObject(eModelName modelName, const glm::vec3& startingPosition)
	: modelName(modelName),
	position(startingPosition),
	AABB(startingPosition, ModelManager::getInstance().getModel(modelName))
{
#ifdef LEVEL_EDITOR
	selected = false;
#endif // LEVEL_EDITOR
}

void GameObject::render(ShaderHandler& shaderHandler) const
{
	ModelManager::getInstance().getModel(modelName).render(shaderHandler, *this);
}

#ifdef RENDER_AABB
void GameObject::renderAABB(ShaderHandler& shaderHandler)
{
	AABB.render(shaderHandler);
}
#endif // RENDER_AABB

#ifdef LEVEL_EDITOR
PlannedGameObject::PlannedGameObject()
	: GameObject(),
	active(false)
{}

void PlannedGameObject::render(ShaderHandler& shaderHandler) const
{
	if (active)
	{
		GameObject::render(shaderHandler);
	}
}
#endif // LEVEL_EDITOR