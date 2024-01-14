
#include <iostream>
#include <SFML/Network.hpp>
#include "SFML/Network/TcpListener.hpp"

//
//std::unique_ptr<Server> Server::create(const sf::IpAddress& ipAddress, unsigned short portNumber)
//{
//	Server* server = new Server;
//	if (server->m_tcpListener.listen(portNumber, ipAddress) == sf::Socket::Done)
//	{
//		server->m_socketSelector.add(server->m_tcpListener);
//		server->m_levelName = "Level1.tmx";
//		if (!XMLParser::loadLevelAsServer(server->m_levelName, server->m_levelSize,
//			server->m_tileManager.m_tileLayers, server->m_tileManager.m_collisionLayer,
//			server->m_spawnPositions, server->m_tileSize))
//		{
//			delete server;
//			return std::unique_ptr<Server>();
//		}
//
//		//Initialize AI Players
//		for (int i = 0; i < MAX_AI_PLAYERS; i++)
//		{
//			assert(!server->m_spawnPositions.empty());
//			sf::Vector2f startingPosition = server->m_spawnPositions.back();
//			server->m_spawnPositions.pop_back();
//
//			int clientID = static_cast<int>(server->m_players.size());
//			server->m_players.emplace_back(std::make_unique<PlayerServerAI>(clientID, startingPosition, *server));
//		}
//
//		PathFinding::getInstance().createGraph(server->m_levelSize);
//		return std::unique_ptr<Server>(server);
//	}
//	else
//	{
//		delete server;
//		return std::unique_ptr<Server>();
//	}
//}

int main()
{
	sf::TcpListener tcp_listener{};
	sf::SocketSelector socket_selector{};



	return 0;
}