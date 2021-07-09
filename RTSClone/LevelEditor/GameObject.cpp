#include "GameObject.h"
#include "ShaderHandler.h"
#include "Globals.h"
#include "Model.h"
#include "ModelManager.h"

GameObject::GameObject(const Model& model)
	: position(),
	rotation(),
	aabb(),
	model(model)
{
	aabb.reset(position, model);
}

GameObject::GameObject(const Model& model, const glm::vec3& startingPosition, glm::vec3 startingRotation)
	: position(startingPosition),
	rotation(startingRotation),
	aabb(),
	model(model)
{
	aabb.reset(position, model);
}

void GameObject::setPosition(const glm::vec3& _position)
{
	position = _position;
	aabb.update(position);
}

void GameObject::render(ShaderHandler& shaderHandler, bool highlight) const
{
	model.get().render(shaderHandler, position, rotation);
}

#ifdef RENDER_AABB
void GameObject::renderAABB(ShaderHandler& shaderHandler)
{
	aabb.render(shaderHandler);
}
#endif // RENDER_AABB