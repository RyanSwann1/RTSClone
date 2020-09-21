#include "UIManager.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#include "GameMessageType.h"
#include "GameEvent.h"
#include "GameEventHandler.h"
#include "Globals.h"
#include <imgui/imgui.h>
#include <string>
#include <sstream>
#include <iomanip>

namespace 
{
	const std::array<std::string, static_cast<size_t>(eEntityType::Max) + 1> ENTITY_NAME_CONVERSIONS =
	{
		"Unit", 
		"Worker",
		"HQ",
		"SupplyDepot",
		"Barracks",
		"Mineral",
		"Projectile"
	};

	const std::array<std::string, static_cast<size_t>(eFactionController::Max) + 1> FACTION_NAME_CONVERSIONS =
	{
		"Player",
		"AI_1",
		"AI_2",
		"AI_3"
	};
}

//PlayerDetailsWidget
PlayerDetailsWidget::PlayerDetailsWidget()
	: Widget()
{}

void PlayerDetailsWidget::render(const sf::Window& window)
{
	if (m_active)
	{
		ImGui::SetNextWindowSize(ImVec2(750, 750), ImGuiCond_FirstUseEver);
		ImGui::Begin("Player", nullptr, ImGuiWindowFlags_NoResize);
		ImGui::Text("Resources:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(m_receivedMessage.resourceAmount).c_str());
		ImGui::Text("Population:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(m_receivedMessage.currentPopulationAmount).c_str());
		ImGui::Text("Max Population:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(m_receivedMessage.maximumPopulationAmount).c_str());
		
		ImGui::End();

		m_active = false;
	}
}

//SelectedEntityWidget
SelectedEntityWidget::SelectedEntityWidget()
	: Widget()
{}

void SelectedEntityWidget::render(const sf::Window& window)
{
	if (m_active)
	{
		ImGui::SetNextWindowPos(ImVec2(500, 650));
		ImGui::SetNextWindowSize(ImVec2(300, 300));
		ImGui::Begin(ENTITY_NAME_CONVERSIONS[static_cast<int>(m_receivedMessage.entityType)].c_str(), nullptr, 
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		if (Globals::UNIT_SPAWNER_TYPES.isMatch(m_receivedMessage.entityType))
		{
			ImGui::Text("Health:");
			ImGui::SameLine();
			ImGui::Text(std::to_string(m_receivedMessage.health).c_str());
			ImGui::Text("Spawn Queue:");
			ImGui::SameLine();
			ImGui::Text(std::to_string(m_receivedMessage.queueSize).c_str());
			if (m_receivedMessage.owningFaction == eFactionController::Player)
			{
				ImGui::Text("Spawn Time:");
				ImGui::SameLine();
				std::stringstream spawnTimeStream;
				spawnTimeStream << std::fixed << std::setprecision(2) << m_receivedMessage.spawnTime;
				ImGui::Text(spawnTimeStream.str().c_str());

				switch (m_receivedMessage.entityType)
				{
				case eEntityType::HQ:
					if (ImGui::Button("Spawn Worker"))
					{
						GameEventHandler::getInstance().addEvent({ eGameEventType::SpawnUnit, eEntityType::Worker, m_receivedMessage.entityID });
					}
					break;
				case eEntityType::Barracks:
					if (ImGui::Button("Spawn Unit"))
					{
						GameEventHandler::getInstance().addEvent({ eGameEventType::SpawnUnit, eEntityType::Unit, m_receivedMessage.entityID });
					}
					break;
				default:
					assert(false);
				}
			}
		}
		else
		{
			ImGui::Text("Health:");
			ImGui::SameLine();
			ImGui::Text(std::to_string(m_receivedMessage.health).c_str());
			if (m_receivedMessage.owningFaction == eFactionController::Player &&
				m_receivedMessage.entityType == eEntityType::Worker)
			{
				if (ImGui::Button("Barracks"))
				{
					GameEventHandler::getInstance().addEvent(
						{ eGameEventType::ActivatePlayerPlannedBuilding, eEntityType::Barracks, m_receivedMessage.entityID });
				}

				if (ImGui::Button("Supply Depot"))
				{
					GameEventHandler::getInstance().addEvent(
						{ eGameEventType::ActivatePlayerPlannedBuilding, eEntityType::SupplyDepot, m_receivedMessage.entityID });
				}
			}
		}

		ImGui::End();
	}
}

//WinningFactionWidget
WinningFactionWidget::WinningFactionWidget()
	: Widget()
{}

void WinningFactionWidget::render(const sf::Window& window)
{
	if (m_active)
	{
		ImGui::Begin("Winning Faction", nullptr);
		ImGui::Text(FACTION_NAME_CONVERSIONS[static_cast<int>(m_receivedMessage.winningFaction)].c_str());
		ImGui::End();
	}
}

//UIManager
UIManager::UIManager()
	: m_playerDetailsWidget(),	
	m_selectedEntityWidget(),
	m_winningFactionWidget()
{
	GameMessenger::getInstance().subscribe<GameMessages::UIDisplayPlayerDetails>([this](const GameMessages::UIDisplayPlayerDetails& gameMessage)
		{ return onDisplayPlayerDetails(gameMessage); }, this);

	GameMessenger::getInstance().subscribe<GameMessages::UIDisplaySelectedEntity>([this](const GameMessages::UIDisplaySelectedEntity& gameMessage)
		{ return onDisplayEntity(gameMessage); }, this);

	GameMessenger::getInstance().subscribe<GameMessages::UIDisplayWinner>([this](const GameMessages::UIDisplayWinner& gameMessage)
		{ return onDisplayWinningFaction(gameMessage); }, this);
	
	GameMessenger::getInstance().subscribe<GameMessages::BaseMessage<eGameMessageType::UIClearDisplaySelectedEntity>>([this](
		const GameMessages::BaseMessage<eGameMessageType::UIClearDisplaySelectedEntity>& gameMessage)
	{ return onClearDisplayEntity(gameMessage); }, this);

	GameMessenger::getInstance().subscribe<GameMessages::BaseMessage<eGameMessageType::UIClearWinner>>([this](
		const GameMessages::BaseMessage<eGameMessageType::UIClearWinner>& gameMessage)
		{ return onClearDisplayWinner(gameMessage); }, this);
}
	
UIManager::~UIManager()
{
	GameMessenger::getInstance().unsubscribe<GameMessages::UIDisplayPlayerDetails>(this);
	GameMessenger::getInstance().unsubscribe<GameMessages::UIDisplaySelectedEntity>(this);
	GameMessenger::getInstance().unsubscribe<GameMessages::UIDisplayWinner>(this);
	GameMessenger::getInstance().unsubscribe<GameMessages::BaseMessage<eGameMessageType::UIClearDisplaySelectedEntity>>(this);
	GameMessenger::getInstance().unsubscribe<GameMessages::BaseMessage<eGameMessageType::UIClearWinner>>(this);
}

void UIManager::render(const sf::Window& window)
{
	m_playerDetailsWidget.render(window);
	m_selectedEntityWidget.render(window);	
	m_winningFactionWidget.render(window);
}

void UIManager::onDisplayPlayerDetails(const GameMessages::UIDisplayPlayerDetails& gameMessage)
{
	m_playerDetailsWidget.set(gameMessage);
}

void UIManager::onDisplayEntity(const GameMessages::UIDisplaySelectedEntity& gameMessage)
{
	m_selectedEntityWidget.set(gameMessage);
}

void UIManager::onClearDisplayEntity(const GameMessages::BaseMessage<eGameMessageType::UIClearDisplaySelectedEntity>& gameMessage)
{
	m_selectedEntityWidget.deactivate();
}

void UIManager::onClearDisplayWinner(const GameMessages::BaseMessage<eGameMessageType::UIClearWinner>& gameMessage)
{
	m_winningFactionWidget.deactivate();
}

void UIManager::onDisplayWinningFaction(const GameMessages::UIDisplayWinner& gameMessage)
{
	m_winningFactionWidget.set(gameMessage);
}