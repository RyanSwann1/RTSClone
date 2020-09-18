#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "EntityType.h"
#include "FactionController.h"
#include "GameMessageType.h"
#include "GameMessages.h"
#include <SFML/Graphics.hpp>

template <class MessageType>
class Widget
{
public:
	void deactivate()
	{
		m_active = false;
	}
	void set(const MessageType& gameMessage)
	{
		m_active = true;
		m_receivedMessage = gameMessage;
	}

protected:
	Widget()
		: m_active(false),
		m_receivedMessage()
	{}
	bool m_active;
	MessageType m_receivedMessage;
};

struct PlayerDetailsWidget : public Widget<GameMessages::UIDisplayPlayerDetails>
{
	PlayerDetailsWidget();
	void render(const sf::Window& window);
};

struct EntityWidget : public Widget<GameMessages::UIDisplayEntity>
{
	EntityWidget();
	void render(const sf::Window& window);
};

struct WinningFaction : public Widget<GameMessages::UIDisplayWinner>
{
	WinningFaction();
	void render(const sf::Window& window);
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