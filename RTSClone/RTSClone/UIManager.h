#pragma once

#include "NonCopyable.h"
#include "NonMovable.h"
#include "EntityType.h"
#include "FactionController.h"
#include "GameMessageType.h"
#include "GameMessages.h"
#include "TargetEntity.h"
#include <SFML/Graphics.hpp>

//https://eliasdaler.github.io/using-imgui-with-sfml-pt2/#getting-back-to-the-context-of-the-window-tree-etc

template <class MessageType>
class Widget : private NonCopyable, private NonMovable
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

struct SelectedEntityWidget : public Widget<GameMessages::UIDisplaySelectedEntity>
{
	SelectedEntityWidget();
	void render(const sf::Window& window);
};

struct WinningFactionWidget : public Widget<GameMessages::UIDisplayWinner>
{
	WinningFactionWidget();
	void render(const sf::Window& window);
};

struct GameEvent;
struct Camera;
class FactionHandler;
class UIManager : private NonCopyable, private NonMovable
{
public:
	UIManager();
	~UIManager();

	void handleInput(const sf::Window& window, const FactionHandler& factionHandler, const Camera& camera,
		const sf::Event& currentSFMLEvent);
	void handleEvent(const GameEvent& gameEvent);
	void update(const FactionHandler& factionHandler);
	void render(const sf::Window& window);

private:
	TargetEntity m_selectedEntity;
	PlayerDetailsWidget m_playerDetailsWidget;
	SelectedEntityWidget m_selectedEntityWidget;
	WinningFactionWidget m_winningFactionWidget;

	void onDisplayPlayerDetails(const GameMessages::UIDisplayPlayerDetails& gameMessage);
	void onDisplayEntity(const GameMessages::UIDisplaySelectedEntity& gameMessage);
	void onDisplayWinningFaction(const GameMessages::UIDisplayWinner& gameMessage);
	void onClearDisplayEntity(const GameMessages::BaseMessage<eGameMessageType::UIClearDisplaySelectedEntity>& gameMessage);
	void onClearDisplayWinner(const GameMessages::BaseMessage<eGameMessageType::UIClearWinner>& gameMessage);
};