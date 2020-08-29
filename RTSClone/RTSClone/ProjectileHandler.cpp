#include "ProjectileHandler.h"
#include "FactionPlayer.h"
#include "FactionAI.h"

ProjectileHandler::ProjectileHandler()
	: m_projectiles()
{}

void ProjectileHandler::addProjectile(const GameEvent& gameEvent)
{
	m_projectiles.emplace_back(gameEvent);
}

void ProjectileHandler::update(float deltaTime, const FactionPlayer& player, const FactionAI& playerAI)
{
	for (auto projectile = m_projectiles.begin(); projectile != m_projectiles.end();)
	{
		projectile->update(deltaTime);
	
		bool projectileCollision = false;
		switch (projectile->m_gameEvent.senderFaction)
		{
		case eFactionName::Player:
		{
			projectileCollision = playerAI.getEntity(projectile->getAABB(), projectile->m_gameEvent.targetID);
		}
			break;
		case eFactionName::AI:
			projectileCollision = player.getEntity(projectile->getAABB(), projectile->m_gameEvent.targetID);
			break;
		}

		if (projectileCollision || projectile->isReachedDestination())
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

#ifdef RENDER_AABB
void ProjectileHandler::renderAABB(ShaderHandler& shaderHandler)
{
	for (auto& projectile : m_projectiles)
	{
		projectile.renderAABB(shaderHandler);
	}
}
#endif // RENDER_AABB