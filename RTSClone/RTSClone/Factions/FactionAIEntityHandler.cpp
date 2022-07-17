#include "FactionAIEntityHandler.h" 
#include "Events/GameEvents.h"

void FactionAIEntityHandler::HandleEvent(const GameEvent& game_event, const Map& map, FactionHandler& faction_handler)
{
	switch (game_event.type)
	{
	case eGameEventType::EntityIdle:
		
		break;
	}
}
