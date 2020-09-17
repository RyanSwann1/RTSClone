#include "ProjectileHandler.h"
#include "Faction.h"
#include "GameEventHandler.h"
#include "FactionHandler.h"

ProjectileHandler::ProjectileHandler()
	: m_projectiles()
{}

void ProjectileHandler::addProjectile(const GameEvent& gameEvent)
{
	m_projectiles.emplace_back(gameEvent);
}

void ProjectileHandler::update(float deltaTime, const FactionHandler& factionHandler)
{
	for (auto projectile = m_projectiles.begin(); projectile != m_projectiles.end();)
	{	
		projectile->update(deltaTime);

		const Faction& targetFaction = factionHandler.getFaction(projectile->getSenderEvent().targetFaction);
		bool projectileCollision = false;
		projectileCollision = targetFaction.getEntity(projectile->getAABB(), projectile->getSenderEvent().targetID);

		if (projectileCollision || projectile->isReachedDestination())
		{
			if (projectileCollision)
			{
				GameEventHandler::getInstance().addEvent({ eGameEventType::Attack, projectile->getSenderEvent().senderFaction,
					projectile->getID(), projectile->getSenderEvent().targetFaction, 
					projectile->getSenderEvent().targetID, projectile->getSenderEvent().damage });
			}

			projectile = m_projectiles.erase(projectile);
		}
		else
		{
			++projectile;
		}
	}
}

void ProjectileHandler::render(ShaderHandler& shaderHandler) const
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