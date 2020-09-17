#include "UIManager.h"
#include "GameMessages.h"
#include "GameMessenger.h"
#include "GameMessageType.h"
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

//Widget
Widget::Widget()
	: active(false)
{}

PlayerDetailsWidget::PlayerDetailsWidget()
	: Widget(),
	resourcesAmount(0),
	currentPopulation(0),
	maxPopulation(0)
{}

void PlayerDetailsWidget::render()
{
	if (active)
	{
		ImGui::Begin("Player", nullptr);
		ImGui::Text("Resources:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(resourcesAmount).c_str());
		ImGui::Text("Population:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(currentPopulation).c_str());
		ImGui::Text("Max Population:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(maxPopulation).c_str());
		
		ImGui::End();

		active = false;
	}
}

EntityWidget::EntityWidget()
	: Widget(),
	entityType(),
	entityHealth(0)
{}

void EntityWidget::render()
{
	if (active)
	{
		ImGui::Begin("Entity", nullptr);
		ImGui::Text(ENTITY_NAME_CONVERSIONS[static_cast<int>(entityType)].c_str());
		ImGui::SameLine();
		ImGui::Text(std::to_string(entityHealth).c_str());
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

void UIManager::render()
{
	m_playerDetailsWidget.render();
	m_entityWidget.render();	
}

void UIManager::onDisplayPlayerDetails(const GameMessages::UIDisplayPlayerDetails& gameMessage)
{
	m_playerDetailsWidget.active = true;
	m_playerDetailsWidget.resourcesAmount = gameMessage.resourceAmount;
	m_playerDetailsWidget.currentPopulation = gameMessage.currentPopulationAmount;
	m_playerDetailsWidget.maxPopulation = gameMessage.maximumPopulationAmount;
}

void UIManager::onDisplayEntity(const GameMessages::UIDisplayEntity& gameMessage)
{
	m_entityWidget.active = true;
	m_entityWidget.entityType = gameMessage.entityType;
	m_entityWidget.entityHealth = gameMessage.entityHealth;
}

void UIManager::onClearDisplayEntity(const GameMessages::BaseMessage<eGameMessageType::UIClearDisplayEntity>& gameMessage)
{
	m_entityWidget.active = false;
}