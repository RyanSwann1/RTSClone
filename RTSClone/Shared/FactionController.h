#pragma once

#include <array>
#include <string>

enum class eFactionController
{
	Player = 0,
	AI_1,
	AI_2,
	AI_3,
	Max = AI_3
};

struct FactionControllerDetails
{
	FactionControllerDetails();
	FactionControllerDetails(eFactionController controller, const std::string& text);

	eFactionController controller;
	std::string text;
};

extern const std::array<FactionControllerDetails, static_cast<size_t>(eFactionController::Max) + 1> FACTION_CONTROLLER_DETAILS;