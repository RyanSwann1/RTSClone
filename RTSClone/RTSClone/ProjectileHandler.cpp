#include "ProjectileHandler.h"

ProjectileHandler::ProjectileHandler()
	: m_projectiles()
{}

void ProjectileHandler::addProjectile(const GameEvent& gameEvent)
{
	m_projectiles.emplace_back(gameEvent);
}

void ProjectileHandler::update(float deltaTime)
{
	for (auto projectile = m_projectiles.begin(); projectile != m_projectiles.end();)
	{
		projectile->update(deltaTime);

		if (projectile->isReachedDestination())
		{
			projectile = m_projectiles.erase(projectile);
		}
		else
		{
			++projectile;
		}
	}
}

void ProjectileHandler::render(ShaderHandler& shaderHandler)
{
	for (const auto& projectile : m_projectiles)
	{
		projectile.render(shaderHandler);
	}
}