#pragma once

#include "EntityType.h"
#include "FactionController.h"
#include "GameMessages.h"
#include "TargetEntity.h"
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
	~UIManager();

	void handleInput(const sf::Window& window, const FactionHandler& factionHandler, const Camera& camera,
		const sf::Event& currentSFMLEvent);
	void handleEvent(const GameEvent& gameEvent);
	void update(const FactionHandler& factionHandler);
	void render(const sf::Window& window);

private:
	PlayerDetailsWidget m_playerDetailsWidget;
	std::unique_ptr<SelectedEntityWidget> m_selectedEntityWidget;
	WinningFactionWidget m_winningFactionWidget;

	void onDisplayPlayerDetails(const GameMessages::UIDisplayPlayerDetails& gameMessage);
	void onDisplayEntity(const GameMessages::UIDisplaySelectedEntity& gameMessage);
	void onDisplayWinningFaction(const GameMessages::UIDisplayWinner& gameMessage);
	void onClearDisplayEntity(GameMessages::UIClearDisplaySelectedEntity message);
	void onClearDisplayWinner(GameMessages::UIClearWinner message);
};