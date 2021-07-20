#include "GameObject.h"
#include "ShaderHandler.h"
#include "Globals.h"
#include "Model.h"
#include "ModelManager.h"

GameObject::GameObject(Model& model)
	: position(0.f),
	rotation(0.f),
	scale(model.scale),
	aabb(position, model),
	model(model),
	useLocalScale(false)
{}

GameObject::GameObject(Model& model, const glm::vec3& startingPosition, glm::vec3 startingRotation)
	: position(startingPosition),
	rotation(startingRotation),
	scale(model.scale),
	aabb(position, model),
	model(model),
	useLocalScale(false)
{}

void GameObject::setPosition(const glm::vec3& _position)
{
	position = _position;
	aabb.update(position);
}

void GameObject::rotate(float y)
{
	rotation.y += y;
	if (glm::abs(y) >= 360.0f)
	{
		rotation = { rotation.x, 0.0f, rotation.z };
	}
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