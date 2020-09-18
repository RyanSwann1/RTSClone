#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "EntityType.h"
#include "FactionController.h"
#include "GameMessageType.h"
#include <SFML/Graphics.hpp>

namespace GameMessages
{
	struct UIDisplayPlayerDetails;
	struct UIDisplayEntity;
	template <eGameMessageType type>
	struct BaseMessage;
};

class Widget
{
public:
	void deactivate();

protected:
	Widget();
	bool m_active;
};

class PlayerDetailsWidget : public Widget
{
public:
	PlayerDetailsWidget();
	
	void set(const GameMessages::UIDisplayPlayerDetails& gameMessage);
	void render(const sf::Window& window);

private:
	int m_resourcesAmount;
	int m_currentPopulation;
	int m_maxPopulation;
};

class EntityWidget : public Widget
{
public:
	EntityWidget();

	void set(const GameMessages::UIDisplayEntity& gameMessage);
	void render(const sf::Window& window);

private:
	eFactionController m_owningFaction;
	int m_entityID;
	eEntityType m_entityType;
	int m_entityHealth;
	int m_unitSpawnerQueueSize;
};

class UIManager : private NonCopyable, private NonMovable
{
public:
	UIManager();
	~UIManager();

	void render(const sf::Window& window);

private:
	PlayerDetailsWidget m_playerDetailsWidget;
	EntityWidget m_entityWidget;

	void onDisplayPlayerDetails(const GameMessages::UIDisplayPlayerDetails& gameMessage);
	void onDisplayEntity(const GameMessages::UIDisplayEntity& gameMessage);
	void onClearDisplayEntity(const GameMessages::BaseMessage<eGameMessageType::UIClearDisplayEntity>& gameMessage);
};