#include "ProjectileHandler.h"
#include "FactionPlayer.h"
#include "FactionAI.h"
#include "GameEventHandler.h"

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
		switch (projectile->getSenderEvent().senderFaction)
		{
		case eFactionName::Player:
			projectileCollision = playerAI.getEntity(projectile->getAABB(), projectile->getSenderEvent().targetID);
			break;
		case eFactionName::AI:
			projectileCollision = player.getEntity(projectile->getAABB(), projectile->getSenderEvent().targetID);
			break;
		}

		if (projectileCollision || projectile->isReachedDestination())
		{
			if (projectileCollision)
			{
				GameEventHandler::getInstance().addEvent({ eGameEventType::Attack, projectile->getSenderEvent().senderFaction,
					projectile->getID(), projectile->getSenderEvent().targetID });
			}

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