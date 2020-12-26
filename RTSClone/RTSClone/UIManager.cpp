#include "UIManager.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#include "GameMessageType.h"
#include "GameEvent.h"
#include "GameEventHandler.h"
#include "FactionHandler.h"
#include "Globals.h"
#include "Camera.h"
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
		"HQ",
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

	void displaySpawnTime(float spawnTime)
	{
		ImGui::Text("Spawn Time:");
		ImGui::SameLine();
		std::stringstream spawnTimeStream;
		spawnTimeStream << std::fixed << std::setprecision(2) << spawnTime;
		ImGui::Text(spawnTimeStream.str().c_str());
	}

	void displaySpawnQueue(int spawnQueueSize)
	{
		ImGui::Text("Spawn Queue:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(spawnQueueSize).c_str());
	}

	void displayBuildTime(float buildTime)
	{
		ImGui::Text("BuildTime:");
		ImGui::SameLine();
		std::stringstream buildTimeStream;
		buildTimeStream << std::fixed << std::setprecision(2) << buildTime;
		ImGui::Text(buildTimeStream.str().c_str());
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
		case eEntityType::HQ:
			if (m_receivedMessage.owningFaction == eFactionController::Player)
			{
				displaySpawnQueue(m_receivedMessage.queueSize);
				displaySpawnTime(m_receivedMessage.spawnTime);

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
				displaySpawnTime(m_receivedMessage.spawnTime);

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
				displayBuildTime(m_receivedMessage.buildTime);

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

void UIManager::handleInput(const sf::Window& window, const FactionHandler& factionHandler, const Camera& camera,
	const sf::Event& currentSFMLEvent)
{
	if (currentSFMLEvent.type == sf::Event::MouseButtonPressed &&
		currentSFMLEvent.mouseButton.button == sf::Mouse::Left)
	{
		glm::vec3 mouseToGroundPosition = camera.getMouseToGroundPosition(window);
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

		if (!selectedEntity)
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
			case eEntityType::HQ:
			case eEntityType::Barracks:
			{
				const UnitSpawnerBuilding& unitSpawnerBuilding = static_cast<const UnitSpawnerBuilding&>(*targetEntity);
				m_selectedEntityWidget.set({ m_selectedEntity.getFactionController(), m_selectedEntity.getID(), targetEntity->getEntityType(),
					unitSpawnerBuilding.getHealth(), unitSpawnerBuilding.getShield(), unitSpawnerBuilding.getCurrentSpawnCount(),
					unitSpawnerBuilding.getSpawnTimer().getExpiredTime() - unitSpawnerBuilding.getSpawnTimer().getElaspedTime() });
			}
			break;
			case eEntityType::Unit:
			case eEntityType::SupplyDepot:
			case eEntityType::Turret:
			case eEntityType::Laboratory:
				m_selectedEntityWidget.set({ m_selectedEntity.getFactionController(), m_selectedEntity.getID(), targetEntity->getEntityType(),
					targetEntity->getHealth(), targetEntity->getShield() });
				break;
			case eEntityType::Worker:
			{
				const Timer& buildTimer = static_cast<const Worker&>(*targetEntity).getBuildTimer();
				m_selectedEntityWidget.set({ m_selectedEntity.getFactionController(), m_selectedEntity.getID(), targetEntity->getEntityType(),
					targetEntity->getHealth(), targetEntity->getShield(), buildTimer.getExpiredTime() - buildTimer.getElaspedTime() });
			}
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