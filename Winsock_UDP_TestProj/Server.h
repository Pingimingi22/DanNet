#pragma once
#include "Peer.h"
#include <vector>
class Server
{
public:
	Server();

	void Init();




private:
	std::vector<Peer*> m_connectedPeers;

};