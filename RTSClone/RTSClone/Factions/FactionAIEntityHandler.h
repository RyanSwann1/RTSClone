#pragma once

struct GameEvent;
class Map;
class FactionHandler;
class FactionAIEntityHandler
{
public:

	void HandleEvent(const GameEvent& game_event, const Map& map, FactionHandler& faction_handler);

private:
};