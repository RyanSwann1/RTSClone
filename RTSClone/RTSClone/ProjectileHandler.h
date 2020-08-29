#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "Projectile.h"
#include <vector>

class ShaderHandler;
class ProjectileHandler : private NonCopyable, private NonMovable
{
public:
	ProjectileHandler();
	
	void addProjectile(const GameEvent& gameEvent);
	void update(float deltaTime);
	void render(ShaderHandler& shaderHandler);

private:
	std::vector<Projectile> m_projectiles;
};