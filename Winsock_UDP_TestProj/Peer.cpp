#include "UDPListener.h"
#include "Peer.h"

#include <string>

#include "Packet.h"



#include <assert.h>

Peer::Peer(bool server = false, unsigned short portNumber = NULL)
{
	if (server)
	{
		server = true;
		assert(portNumber != NULL); // If they pass in 0 for the port, that's an error. because server's need well known ports.

		m_udpListener = UDPListener("*", std::to_string(portNumber));
	}

}

Peer Peer::CreatePeer(bool server, unsigned short portNumber)
{
	// eh maybe i'll make this a thing one day. The constructor is fine for now.
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

void Peer::UDPSend(Packet packet)
{
	//int sendResult = sendto(m_hostSocket)
}

void Peer::UDPSendReliable(Packet packet)
{
	// TODO actually make this thing.
}

void Peer::Update()
{
	while (m_udpListener.IsRunning())
	{
		m_udpListener.Update();
	}
}
