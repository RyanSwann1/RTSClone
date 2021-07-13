#include "GameObject.h"
#include "ShaderHandler.h"
#include "Globals.h"
#include "Model.h"
#include "ModelManager.h"

GameObject::GameObject(Model& model)
	: position(),
	rotation(),
	scale(model.scale),
	aabb(),
	model(model),
	useLocalScale(false)
{
	aabb.reset(position, model);
}

GameObject::GameObject(Model& model, const glm::vec3& startingPosition, glm::vec3 startingRotation)
	: position(startingPosition),
	rotation(startingRotation),
	scale(model.scale),
	aabb(),
	model(model),
	useLocalScale(false)
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
	model.get().render(shaderHandler, *this, highlight);
}

#ifdef RENDER_AABB
void GameObject::renderAABB(ShaderHandler& shaderHandler)
{
	aabb.render(shaderHandler);
}
#endif // RENDER_AABB