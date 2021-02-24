#pragma once

#include "Projectile.h"
#include <vector>
#include <memory>

class ShaderHandler;
class FactionHandler;
class ProjectileHandler
{
public:
	ProjectileHandler();
	ProjectileHandler(const ProjectileHandler&) = delete;
	ProjectileHandler& operator=(const ProjectileHandler&) = delete;
	ProjectileHandler(ProjectileHandler&&) = delete;
	ProjectileHandler& operator=(ProjectileHandler&&) = delete;
	
	void addProjectile(const GameEvent& gameEvent);
	void update(float deltaTime, const FactionHandler& factionHandler);
	void render(ShaderHandler& shaderHandler) const;

#ifdef RENDER_AABB
	void renderAABB(ShaderHandler& shaderHandler);
#endif // RENDER_AABB

private:
	std::vector<Projectile> m_projectiles;
};