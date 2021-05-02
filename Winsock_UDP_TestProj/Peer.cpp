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

		m_udpListener = UDPListener(this, std::to_string(portNumber), "*");
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

/// <summary>
/// This will allow peer's to connect to a specific IPv4 and port. This will be useful for clients when they want to send messages back to the server.
/// </summary>
/// <param name="ipAddress"></param>
/// <param name="portNumber"></param>
void Peer::Connect(std::string ipAddress, unsigned short portNumber)
{
	sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(sockaddr_in));

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);
	inet_pton(AF_INET, &ipAddress[0], &serverAddress.sin_addr.S_un.S_addr);

	std::cout << "Connected to " << ipAddress << " on port: " << portNumber << std::endl;

	// ============ TODO ============= MAKE IT SO THE SERVER GETS A CONNECTION TO THE CLIENT.

	m_serverConnection = serverAddress;
}

/// <summary>
/// ReceivePacket() will return null if no packet has been received. Otherwise it returns a packet.
/// </summary>
/// <returns></returns>
char* Peer::UDPReceiveBytes()
{
	
}

void Peer::UDPSend(Packet packet)
{
	//m_udpListener.Send(packet);
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
