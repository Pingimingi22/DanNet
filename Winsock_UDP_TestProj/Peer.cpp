#include "UDPListener.h"
#include "Peer.h"

#include <string>

#include "Packet.h"



#include <assert.h>

Peer::Peer()
{
}

Peer Peer::CreatePeer(bool server, unsigned short portNumber)
{
	if (server)
	{
		server = true;
		assert(portNumber != NULL); // If they pass in 0 for the port, that's an error.

		m_udpListener = UDPListener("*", std::to_string(portNumber));
	}

	Peer ehtestpeer;	// ======================== TODO actually make this a thing ======================== // 
	return ehtestpeer;
}

void Peer::StartPeer()
{
	m_udpListener.Start();
}

void Peer::ShutdownPeer()
{
}

Packet Peer::GetCurrentPacket()
{
	return Packet();
}

void Peer::Update()
{
	while (m_udpListener.IsRunning())
	{
		m_udpListener.Update();
	}
}
