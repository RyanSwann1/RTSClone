#include "Core/FactionController.h"
#include "Core/Globals.h"

FactionControllerDetails::FactionControllerDetails()
	: controller(),
	text()
{}

FactionControllerDetails::FactionControllerDetails(eFactionController controller, const std::string& text)
	: controller(controller),
	text(text)
{}

const std::array<FactionControllerDetails, static_cast<size_t>(eFactionController::Max) + 1> FACTION_CONTROLLER_DETAILS =
{
	FactionControllerDetails(eFactionController::Player, Globals::TEXT_HEADER_BEGINNING + "Player"),
	FactionControllerDetails(eFactionController::AI_1, Globals::TEXT_HEADER_BEGINNING + "PlayerAI_1"),
	FactionControllerDetails(eFactionController::AI_2, Globals::TEXT_HEADER_BEGINNING + "PlayerAI_2"),
	FactionControllerDetails(eFactionController::AI_3, Globals::TEXT_HEADER_BEGINNING + "PlayerAI_3")
};