#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "EntityType.h"
#include "GameMessageType.h"

namespace GameMessages
{
	struct UIDisplayPlayerDetails;
	struct UIDisplayEntity;
	template <eGameMessageType type>
	struct BaseMessage;
};

struct Widget
{
	Widget();

	bool active;
};

struct PlayerDetailsWidget : public Widget
{
	PlayerDetailsWidget();
	
	void render();

	int resourcesAmount;
	int currentPopulation;
	int maxPopulation;
};

struct EntityWidget : public Widget
{
	EntityWidget();

	void render();

	eEntityType entityType;
	int entityHealth;
};

class UIManager : private NonCopyable, private NonMovable
{
public:
	UIManager();
	~UIManager();

	void render();

private:
	PlayerDetailsWidget m_playerDetailsWidget;
	EntityWidget m_entityWidget;

	void onDisplayPlayerDetails(const GameMessages::UIDisplayPlayerDetails& gameMessage);
	void onDisplayEntity(const GameMessages::UIDisplayEntity& gameMessage);
	void onClearDisplayEntity(const GameMessages::BaseMessage<eGameMessageType::UIClearDisplayEntity>& gameMessage);
};