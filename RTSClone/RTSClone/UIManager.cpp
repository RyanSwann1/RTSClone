#include "UIManager.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#include "GameEvents.h"
#include "GameEventHandler.h"
#include "FactionHandler.h"
#include "Globals.h"
#include "Camera.h"
#include "FactionPlayer.h"
#include <imgui/imgui.h>
#include <string>
#include <sstream>
#include <iomanip>

namespace 
{
	//Follows order of eEntityType
	const std::array<std::string, static_cast<size_t>(eEntityType::Max) + 1> ENTITY_NAME_CONVERSIONS =
	{
		"Unit",
		"Worker",
		"Headquarters",
		"SupplyDepot",
		"Barracks",
		"Turret",
		"Laboratory"
	};

	const std::array<std::string, static_cast<size_t>(eFactionController::Max) + 1> FACTION_NAME_CONVERSIONS =
	{
		"Player",
		"AI_1",
		"AI_2",
		"AI_3"
	};

	void displayHealth(int health)
	{
		ImGui::Text("Health:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(health).c_str());
	}

	void displayShield(int shield)
	{
		ImGui::Text("Shield:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(shield).c_str());
	}

	void displaySpawnQueue(int spawnQueueSize)
	{
		ImGui::Text("Spawn Queue:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(spawnQueueSize).c_str());
	}

	void displayResources(int resources)
	{
		ImGui::Text("Resources:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(resources).c_str());
	}

	void displayPopulation(int population)
	{
		ImGui::Text("Population:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(population).c_str());
	}

	void displayMaxPopulation(int maxPopulation)
	{
		ImGui::Text("Max Population:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(maxPopulation).c_str());
	}
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
		displayResources(m_receivedMessage.resourceAmount);
		displayPopulation(m_receivedMessage.currentPopulationAmount);
		displayMaxPopulation(m_receivedMessage.maximumPopulationAmount);
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
		assert(static_cast<size_t>(m_receivedMessage.entityType) < ENTITY_NAME_CONVERSIONS.size());
		ImGui::Begin(ENTITY_NAME_CONVERSIONS[static_cast<size_t>(m_receivedMessage.entityType)].c_str(), nullptr,
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		displayHealth(m_receivedMessage.health);
		displayShield(m_receivedMessage.shield);
		switch (m_receivedMessage.entityType)
		{
		case eEntityType::Headquarters:
			if (m_receivedMessage.owningFaction == eFactionController::Player)
			{
				displaySpawnQueue(m_receivedMessage.queueSize);

				if (ImGui::Button("Spawn Worker"))
				{
					GameEventHandler::getInstance().gameEvents.push(GameEvent::createPlayerSpawnUnit(
						eEntityType::Worker, m_receivedMessage.entityID));
				}
			}
			break;
		case eEntityType::Barracks:
			if (m_receivedMessage.owningFaction == eFactionController::Player)
			{
				displaySpawnQueue(m_receivedMessage.queueSize);

				if (ImGui::Button("Spawn Unit"))
				{
					GameEventHandler::getInstance().gameEvents.push(GameEvent::createPlayerSpawnUnit(
						eEntityType::Unit, m_receivedMessage.entityID));
				}
			}
			break;
		case eEntityType::Worker:
			if (m_receivedMessage.owningFaction == eFactionController::Player)
			{
				if (ImGui::Button("Headquarters"))
				{
					GameEventHandler::getInstance().gameEvents.push(GameEvent::createPlayerActivatePlannedBuilding(
						eEntityType::Headquarters, m_receivedMessage.entityID));
				}
				if (ImGui::Button("Barracks"))
				{
					GameEventHandler::getInstance().gameEvents.push(GameEvent::createPlayerActivatePlannedBuilding(
						eEntityType::Barracks, m_receivedMessage.entityID));
				}
				if (ImGui::Button("Supply Depot"))
				{
					GameEventHandler::getInstance().gameEvents.push(GameEvent::createPlayerActivatePlannedBuilding(
						eEntityType::SupplyDepot, m_receivedMessage.entityID));
				}
				if (ImGui::Button("Turret"))
				{
					GameEventHandler::getInstance().gameEvents.push(GameEvent::createPlayerActivatePlannedBuilding(
						eEntityType::Turret, m_receivedMessage.entityID));
				}
				if (ImGui::Button("Laboratory"))
				{
					GameEventHandler::getInstance().gameEvents.push(GameEvent::createPlayerActivatePlannedBuilding(
						eEntityType::Laboratory, m_receivedMessage.entityID));
				}
			}
			break;
		case eEntityType::Laboratory:
			if (m_receivedMessage.owningFaction == eFactionController::Player)
			{
				if (ImGui::Button("Upgrade Shield"))
				{
					GameEventHandler::getInstance().gameEvents.push(GameEvent::createIncreaseFactionShield(
						m_receivedMessage.owningFaction));
				}
			}
			break;
		case eEntityType::SupplyDepot:
		case eEntityType::Turret:
		case eEntityType::Unit:
			break;
		default:
			assert(false);
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
	: m_selectedEntity(),
	m_playerDetailsWidget(),	
	m_selectedEntityWidget(),
	m_winningFactionWidget()
{
	subscribeToMessenger<GameMessages::UIDisplayPlayerDetails>([this](const GameMessages::UIDisplayPlayerDetails& gameMessage)
		{ return onDisplayPlayerDetails(gameMessage); }, this);

	subscribeToMessenger<GameMessages::UIDisplaySelectedEntity>([this](const GameMessages::UIDisplaySelectedEntity& gameMessage)
		{ return onDisplayEntity(gameMessage); }, this);

	subscribeToMessenger<GameMessages::UIDisplayWinner>([this](const GameMessages::UIDisplayWinner& gameMessage)
		{ return onDisplayWinningFaction(gameMessage); }, this);

	subscribeToMessenger<GameMessages::UIClearDisplaySelectedEntity>([this](
		GameMessages::UIClearDisplaySelectedEntity gameMessage) { return onClearDisplayEntity(gameMessage); }, this);

	subscribeToMessenger<GameMessages::UIClearWinner>([this](
		GameMessages::UIClearWinner gameMessage) { return onClearDisplayWinner(gameMessage); }, this);
}
	
UIManager::~UIManager()
{
	unsubscribeToMessenger<GameMessages::UIDisplayPlayerDetails>(this);
	unsubscribeToMessenger<GameMessages::UIDisplaySelectedEntity>(this);
	unsubscribeToMessenger<GameMessages::UIDisplayWinner>(this);
	unsubscribeToMessenger<GameMessages::UIClearDisplaySelectedEntity>(this);
	unsubscribeToMessenger<GameMessages::UIClearWinner>(this);
}

void UIManager::handleInput(const sf::Window& window, const FactionHandler& factionHandler, const Camera& camera,
	const sf::Event& currentSFMLEvent)
{
	if (currentSFMLEvent.type == sf::Event::MouseButtonPressed &&
		currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
	{
		glm::vec3 mouseToGroundPosition = camera.getRayToGroundPlaneIntersection(window);
		const Entity* selectedEntity = nullptr;
		for (const auto& faction : factionHandler.getFactions())
		{
			if (faction)
			{
				selectedEntity = faction->getEntity(mouseToGroundPosition);
				if (selectedEntity)
				{
					m_selectedEntity.set(faction->getController(), selectedEntity->getID());
					break;
				}
			}
		}

		if (!selectedEntity && 
			factionHandler.isFactionActive(eFactionController::Player) &&
			static_cast<const FactionPlayer&>(factionHandler.getFaction(eFactionController::Player)).getSelectedEntities().size() != 1)
		{
			m_selectedEntity.reset();
		}
	}
}

void UIManager::handleEvent(const GameEvent& gameEvent)
{
	switch (gameEvent.type)
	{
	case eGameEventType::SetTargetEntityGUI:
		m_selectedEntity.set(gameEvent.data.setTargetEntityGUI.factionController,
			gameEvent.data.setTargetEntityGUI.entityID);
		break;
	case eGameEventType::ResetTargetEntityGUI:
		m_selectedEntity.reset();
		break;
	}
}

void UIManager::update(const FactionHandler& factionHandler)
{
	if (m_selectedEntity.getID() != Globals::INVALID_ENTITY_ID &&
		factionHandler.isFactionActive(m_selectedEntity.getFactionController()))
	{
		const Entity* targetEntity = factionHandler.getFaction(m_selectedEntity.getFactionController()).getEntity(m_selectedEntity.getID());
		if (!targetEntity)
		{
			m_selectedEntity.reset();
			m_selectedEntityWidget.deactivate();
		}
		else
		{
			switch (targetEntity->getEntityType())
			{
			case eEntityType::Headquarters:
			case eEntityType::Barracks:
			{
				const EntitySpawnerBuilding& unitSpawnerBuilding = static_cast<const EntitySpawnerBuilding&>(*targetEntity);
				m_selectedEntityWidget.set({ m_selectedEntity.getFactionController(), m_selectedEntity.getID(), targetEntity->getEntityType(),
					unitSpawnerBuilding.getHealth(), unitSpawnerBuilding.getShield(), unitSpawnerBuilding.getCurrentSpawnCount() });
			}
			break;
			case eEntityType::Unit:
			case eEntityType::SupplyDepot:
			case eEntityType::Turret:
			case eEntityType::Laboratory:
			case eEntityType::Worker:
				m_selectedEntityWidget.set({ m_selectedEntity.getFactionController(), m_selectedEntity.getID(), targetEntity->getEntityType(),
					targetEntity->getHealth(), targetEntity->getShield() });
				break;
			break;
			default:
				assert(false);
			}
		}
	}
	else
	{
		m_selectedEntity.reset();
		m_selectedEntityWidget.deactivate();
	}
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

void UIManager::onClearDisplayEntity(GameMessages::UIClearDisplaySelectedEntity)
{
	m_selectedEntityWidget.deactivate();
}

void UIManager::onClearDisplayWinner(GameMessages::UIClearWinner)
{
	m_winningFactionWidget.deactivate();
}

void UIManager::onDisplayWinningFaction(const GameMessages::UIDisplayWinner& gameMessage)
{
	m_winningFactionWidget.set(gameMessage);
}