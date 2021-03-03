#include "GameMessages.h"
#include "Entity.h"

GameMessages::UIDisplayPlayerDetails::UIDisplayPlayerDetails()
	: resourceAmount(0),
	currentPopulationAmount(0),
	maximumPopulationAmount(0)
{}

GameMessages::UIDisplayPlayerDetails::UIDisplayPlayerDetails(int resourceAmount, int currentPopulationAmount, int maximumPopulationAmount)
	: resourceAmount(resourceAmount),
	currentPopulationAmount(currentPopulationAmount),
	maximumPopulationAmount(maximumPopulationAmount)
{}

GameMessages::UIDisplaySelectedEntity::UIDisplaySelectedEntity()
	: owningFaction(),
	entityID(Globals::INVALID_ENTITY_ID),
	entityType(),
	health(0),
	shield(0),
	queueSize(0)
{}

GameMessages::UIDisplaySelectedEntity::UIDisplaySelectedEntity(eFactionController owningFaction, int entityID, eEntityType entityType, 
	int health, int shield)
	: owningFaction(owningFaction),
	entityID(entityID),
	entityType(entityType),
	health(health),
	shield(shield),
	queueSize(0)
{}

GameMessages::UIDisplaySelectedEntity::UIDisplaySelectedEntity(eFactionController owningFaction, int entityID, eEntityType entityType, 
	int health, int shield, int queueSize)
	: owningFaction(owningFaction),
	entityID(entityID),
	entityType(entityType),
	health(health),
	shield(shield),
	queueSize(queueSize)
{}

GameMessages::UIDisplayWinner::UIDisplayWinner()
	: winningFaction()
{}

GameMessages::UIDisplayWinner::UIDisplayWinner(eFactionController winningFaction)
	: winningFaction(winningFaction)
{}
