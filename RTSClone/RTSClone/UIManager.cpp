#include "UIManager.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#include "GameEvents.h"
#include "Factions/FactionHandler.h"
#include "Globals.h"
#include "Camera.h"
#include "Factions/FactionPlayer.h"
#include "Mineral.h"
#include "Level.h"
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
SelectedEntityWidget::SelectedEntityWidget(eFactionController factionController, int ID, eEntityType entityType)
	: Widget(),
	factionController(factionController),
	ID(ID),
	entityType(entityType)
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
					Level::add_event(GameEvent::create<PlayerSpawnEntity>({ eEntityType::Worker, m_receivedMessage.entityID }));
				}
			}
			break;
		case eEntityType::Barracks:
			if (m_receivedMessage.owningFaction == eFactionController::Player)
			{
				displaySpawnQueue(m_receivedMessage.queueSize);

				if (ImGui::Button("Spawn Unit"))
				{
					Level::add_event(GameEvent::create<PlayerSpawnEntity>({ eEntityType::Unit, m_receivedMessage.entityID }));
				}
			}
			break;
		case eEntityType::Worker:
			if (m_receivedMessage.owningFaction == eFactionController::Player)
			{
				if (ImGui::Button(ENTITY_NAME_CONVERSIONS[static_cast<int>(eEntityType::Headquarters)].c_str()))
				{
					Level::add_event(GameEvent::create<PlayerActivatePlannedBuildingEvent>({ eEntityType::Headquarters, m_receivedMessage.entityID }));
				}
				if (ImGui::Button(ENTITY_NAME_CONVERSIONS[static_cast<int>(eEntityType::Barracks)].c_str()))
				{
					Level::add_event(GameEvent::create<PlayerActivatePlannedBuildingEvent>({ eEntityType::Barracks, m_receivedMessage.entityID }));
				}
				if (ImGui::Button(ENTITY_NAME_CONVERSIONS[static_cast<int>(eEntityType::SupplyDepot)].c_str()))
				{
					Level::add_event(GameEvent::create<PlayerActivatePlannedBuildingEvent>({ eEntityType::SupplyDepot, m_receivedMessage.entityID }));
				}
				if (ImGui::Button(ENTITY_NAME_CONVERSIONS[static_cast<int>(eEntityType::Turret)].c_str()))
				{
					Level::add_event(GameEvent::create<PlayerActivatePlannedBuildingEvent>({ eEntityType::Turret, m_receivedMessage.entityID }));
				}
				if (ImGui::Button(ENTITY_NAME_CONVERSIONS[static_cast<int>(eEntityType::Laboratory)].c_str()))
				{
					Level::add_event(GameEvent::create<PlayerActivatePlannedBuildingEvent>({ eEntityType::Laboratory, m_receivedMessage.entityID }));
				}
			}
			break;
		case eEntityType::Laboratory:
			if (m_receivedMessage.owningFaction == eFactionController::Player)
			{
				if (ImGui::Button("Upgrade Shield"))
				{
					Level::add_event(GameEvent::create<IncreaseFactionShieldEvent>({ m_receivedMessage.owningFaction }));
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

//SelectedMineralWidget
SelectedMineralWidget::SelectedMineralWidget(const Mineral& mineral)
	: Widget({ mineral }, true),
	mineral(mineral)
{}

void SelectedMineralWidget::render(const sf::Window& window)
{
	if (m_active)
	{
		ImGui::Begin("Selected Mineral", nullptr);
		ImGui::Text(std::to_string(mineral.getQuantity()).c_str());
		ImGui::End();
	}
}

//UIManager
UIManager::UIManager()
	: m_playerDetailsWidget(),	
	m_selectedEntityWidget(),
	m_selectedMineralWidget(),
	m_winningFactionWidget(),
	m_onDisplayPlayerDetailsID([this](GameMessages::UIDisplayPlayerDetails&& gameMessage) { return onDisplayPlayerDetails(std::move(gameMessage)); }),
	m_onDisplayEntityID([this](GameMessages::UIDisplaySelectedEntity&& gameMessage) { return onDisplayEntity(std::move(gameMessage)); }),
	m_onDisplayMineralID([this](GameMessages::UIDisplaySelectedMineral&& gameMessage) { return onDisplayMineral(std::move(gameMessage)); }),
	m_onDisplayWinningFactionID([this](GameMessages::UIDisplayWinner&& gameMessage){ return onDisplayWinningFaction(std::move(gameMessage)); }),
	m_onClearDisplayEntityID([this](GameMessages::UIClearDisplaySelectedEntity&& gameMessage) { return onClearDisplayEntity(std::move(gameMessage)); }),
	m_onClearSelectedMineralID([this](GameMessages::UIClearSelectedMineral&& gameMessage) { return onClearSelectedMineral(std::move(gameMessage)); }),
	m_onClearDisplayWinnerID([this](GameMessages::UIClearWinner&& gameMessage) { return onClearDisplayWinner(std::move(gameMessage)); })
{}
	
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
					m_selectedEntityWidget = 
						std::make_unique<SelectedEntityWidget>(faction->getController(), selectedEntity->getID(), selectedEntity->getEntityType());
					break;
				}
			}
		}

		if (const FactionPlayer* factionPlayer; 
			!selectedEntity 
			&& (factionPlayer = factionHandler.getFactionPlayer())
			&& factionPlayer->getSelectedEntities().size() != 1)
		{
			m_selectedEntityWidget.reset();
		}
	}
}

void UIManager::handleEvent(const GameEvent& gameEvent)
{
	switch (gameEvent.type)
	{
	case eGameEventType::SetTargetEntityGUI:
		m_selectedEntityWidget = std::make_unique<SelectedEntityWidget>(
			gameEvent.data.setTargetEntityGUI.factionController,
			gameEvent.data.setTargetEntityGUI.entityID, 
			gameEvent.data.setTargetEntityGUI.entityType);
		break;
	case eGameEventType::ResetTargetEntityGUI:
		m_selectedEntityWidget.reset();
		break;
	}
}

void UIManager::update(const FactionHandler& factionHandler)
{
	if (m_selectedEntityWidget)
	{
		if (const Faction* faction = factionHandler.getFaction(m_selectedEntityWidget->factionController))
		{
			const Entity* targetEntity = faction->getEntity(m_selectedEntityWidget->ID, m_selectedEntityWidget->entityType);
			if (!targetEntity)
			{
				m_selectedEntityWidget.reset();
			}
			else
			{
				switch (targetEntity->getEntityType())
				{
				case eEntityType::Headquarters:
				case eEntityType::Barracks:
				{
					const EntitySpawnerBuilding& unitSpawnerBuilding = static_cast<const EntitySpawnerBuilding&>(*targetEntity);
					m_selectedEntityWidget->set({ m_selectedEntityWidget->factionController, m_selectedEntityWidget->ID, targetEntity->getEntityType(),
						unitSpawnerBuilding.getHealth(), unitSpawnerBuilding.getShield(), unitSpawnerBuilding.getCurrentSpawnCount() });
				}
				break;
				case eEntityType::Unit:
				case eEntityType::SupplyDepot:
				case eEntityType::Turret:
				case eEntityType::Laboratory:
				case eEntityType::Worker:
					m_selectedEntityWidget->set({ m_selectedEntityWidget->factionController, m_selectedEntityWidget->ID, targetEntity->getEntityType(),
						targetEntity->getHealth(), targetEntity->getShield() });
					break;
				default:
					assert(false);
				}
			}
		}
		else
		{
			m_selectedEntityWidget.reset();
		}
	}
}

void UIManager::render(const sf::Window& window)
{
	m_playerDetailsWidget.render(window);
	if (m_selectedEntityWidget)
	{
		m_selectedEntityWidget->render(window);
	}
	if (m_selectedMineralWidget)
	{
		m_selectedMineralWidget->render(window);
	}
	m_winningFactionWidget.render(window);
}

void UIManager::onDisplayPlayerDetails(GameMessages::UIDisplayPlayerDetails&& gameMessage)
{
	m_playerDetailsWidget.set(gameMessage);
}

void UIManager::onDisplayEntity(GameMessages::UIDisplaySelectedEntity&& gameMessage)
{
	m_selectedEntityWidget = std::make_unique<SelectedEntityWidget>(gameMessage.owningFaction, gameMessage.entityID, gameMessage.entityType);
}

void UIManager::onClearDisplayEntity(GameMessages::UIClearDisplaySelectedEntity&&)
{
	m_selectedEntityWidget.reset();
}

void UIManager::onClearSelectedMineral(GameMessages::UIClearSelectedMineral&& message)
{
	m_selectedMineralWidget.reset();
}

void UIManager::onClearDisplayWinner(GameMessages::UIClearWinner&&)
{
	m_winningFactionWidget.deactivate();
}

void UIManager::onDisplayWinningFaction(GameMessages::UIDisplayWinner&& gameMessage)
{
	m_winningFactionWidget.set(gameMessage);
}

void UIManager::onDisplayMineral(GameMessages::UIDisplaySelectedMineral&& gameMessage)
{
	m_selectedMineralWidget = std::make_unique<SelectedMineralWidget>(gameMessage.mineral);
}