#include "UIManager.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#include "GameMessageType.h"
#include "Globals.h"
#include <imgui/imgui.h>
#include <string>

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
}

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

EntityWidget::EntityWidget()
	: Widget()
{}

void EntityWidget::render(const sf::Window& window)
{
	if (m_active)
	{
		ImGui::Begin("Entity", nullptr);
		if (!Globals::UNIT_SPAWNER_TYPES.isMatch(m_receivedMessage.entityType))
		{
			ImGui::Text(ENTITY_NAME_CONVERSIONS[static_cast<int>(m_receivedMessage.entityType)].c_str());
			ImGui::SameLine();
			ImGui::Text(std::to_string(m_receivedMessage.health).c_str());
		}
		else
		{
			ImGui::Text(ENTITY_NAME_CONVERSIONS[static_cast<int>(m_receivedMessage.entityType)].c_str());
			ImGui::Text(std::to_string(m_receivedMessage.health).c_str());
			ImGui::Text("Queue");
			ImGui::Text(std::to_string(m_receivedMessage.queueSize).c_str());
		}

		ImGui::End();
	}
}

UIManager::UIManager()
	: m_playerDetailsWidget(),
	m_entityWidget()
{
	GameMessenger::getInstance().subscribe<GameMessages::UIDisplayPlayerDetails>([this](const GameMessages::UIDisplayPlayerDetails& gameMessage)
		{return onDisplayPlayerDetails(gameMessage); }, this);

	GameMessenger::getInstance().subscribe<GameMessages::UIDisplayEntity>([this](const GameMessages::UIDisplayEntity& gameMessage)
		{ return onDisplayEntity(gameMessage); }, this);

	GameMessenger::getInstance().subscribe<GameMessages::BaseMessage<eGameMessageType::UIClearDisplayEntity>>([this](
		const GameMessages::BaseMessage<eGameMessageType::UIClearDisplayEntity>& gameMessage)
		{return onClearDisplayEntity(gameMessage); }, this);
}

UIManager::~UIManager()
{
	GameMessenger::getInstance().unsubscribe<GameMessages::UIDisplayPlayerDetails>(this);
	GameMessenger::getInstance().unsubscribe<GameMessages::UIDisplayEntity>(this);
	GameMessenger::getInstance().unsubscribe<GameMessages::BaseMessage<eGameMessageType::UIClearDisplayEntity>>(this);
}

void UIManager::render(const sf::Window& window)
{
	m_playerDetailsWidget.render(window);
	m_entityWidget.render(window);	
}

void UIManager::onDisplayPlayerDetails(const GameMessages::UIDisplayPlayerDetails& gameMessage)
{
	m_playerDetailsWidget.set(gameMessage);
}

void UIManager::onDisplayEntity(const GameMessages::UIDisplayEntity& gameMessage)
{
	m_entityWidget.set(gameMessage);
}

void UIManager::onClearDisplayEntity(const GameMessages::BaseMessage<eGameMessageType::UIClearDisplayEntity>& gameMessage)
{
	m_entityWidget.deactivate();
}