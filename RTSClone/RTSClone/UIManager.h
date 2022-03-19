#pragma once

#include "EntityType.h"
#include "FactionController.h"
#include "GameMessages.h"
#include "TargetEntity.h"
#include "GameMessenger.h"
#include <SFML/Graphics.hpp>
#include <memory>

//https://eliasdaler.github.io/using-imgui-with-sfml-pt2/#getting-back-to-the-context-of-the-window-tree-etc

template <class MessageType>
class Widget 
{
public:
	Widget(const Widget&) = delete;
	Widget& operator=(const Widget&) = delete;
	Widget(Widget&&) = delete;
	Widget& operator=(Widget&&) = delete;

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
	Widget(bool active = false)
		: m_active(active),
		m_receivedMessage()
	{}
	Widget(const MessageType& message, bool active = false)
		: m_active(active),
		m_receivedMessage(message)
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
	SelectedEntityWidget(eFactionController factionController, int ID, eEntityType entityType);

	void render(const sf::Window& window);

	const eFactionController factionController; 
	const int ID;
	const eEntityType entityType;
};

struct WinningFactionWidget : public Widget<GameMessages::UIDisplayWinner>
{
	WinningFactionWidget();
	void render(const sf::Window& window);
};

class Mineral;
struct SelectedMineralWidget : public Widget<GameMessages::UIDisplaySelectedMineral>
{
	SelectedMineralWidget(const Mineral& mineral);
	void render(const sf::Window& window);
	const Mineral& mineral;
};

struct GameEvent;
struct Camera;
class FactionHandler;
class UIManager 
{
public:
	UIManager();
	UIManager(const UIManager&) = delete;
	UIManager& operator=(const UIManager&) = delete;
	UIManager(UIManager&&) = delete;
	UIManager& operator=(UIManager&&) = delete;

	void handleInput(const sf::Window& window, const FactionHandler& factionHandler, const Camera& camera,
		const sf::Event& currentSFMLEvent);
	void handleEvent(const GameEvent& gameEvent);
	void update(const FactionHandler& factionHandler);
	void render(const sf::Window& window);

private:
	PlayerDetailsWidget m_playerDetailsWidget;
	std::unique_ptr<SelectedEntityWidget> m_selectedEntityWidget;
	std::unique_ptr<SelectedMineralWidget> m_selectedMineralWidget;
	WinningFactionWidget m_winningFactionWidget;	
	BroadcasterSub<GameMessages::UIDisplayPlayerDetails> m_onDisplayPlayerDetailsID;
	BroadcasterSub<GameMessages::UIDisplaySelectedEntity> m_onDisplayEntityID;
	BroadcasterSub<GameMessages::UIDisplayWinner> m_onDisplayWinningFactionID;
	BroadcasterSub<GameMessages::UIDisplaySelectedMineral> m_onDisplayMineralID;
	BroadcasterSub<GameMessages::UIClearDisplaySelectedEntity> m_onClearDisplayEntityID;
	BroadcasterSub<GameMessages::UIClearSelectedMineral> m_onClearSelectedMineralID;
	BroadcasterSub<GameMessages::UIClearWinner> m_onClearDisplayWinnerID;

	void onDisplayPlayerDetails(GameMessages::UIDisplayPlayerDetails&& gameMessage);
	void onDisplayEntity(GameMessages::UIDisplaySelectedEntity&& gameMessage);
	void onDisplayWinningFaction(GameMessages::UIDisplayWinner&& gameMessage);
	void onDisplayMineral(GameMessages::UIDisplaySelectedMineral&& gameMessage);
	void onClearDisplayEntity(GameMessages::UIClearDisplaySelectedEntity&& message);
	void onClearSelectedMineral(GameMessages::UIClearSelectedMineral&& message);
	void onClearDisplayWinner(GameMessages::UIClearWinner&& message);
};