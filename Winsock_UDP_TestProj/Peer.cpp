#include "UDPListener.h"
#include "Peer.h"

#include <string>

#include "Packet.h"



#include <assert.h>

Peer::Peer(bool server, unsigned short portNumber)
{
	WSADATA wsadata;
	int result = WSAStartup(MAKEWORD(2, 2), &wsadata);

	if (result != 0)
	{
		std::cout << "Error occured on WSAStartup()" << std::endl;
	}

	if (server)
	{
		m_isServer = true;
		assert(portNumber != NULL); // If they pass in 0 for the port, that's an error. because server's need well known ports.

		m_udpListener = UDPListener(this, std::to_string(portNumber), "*");
	}
	else
	{
		m_isServer = false;

		m_udpListener = UDPListener(this); // since it's the client we don't have to specify a port.

	}

}

Peer::~Peer()
{
	// TODO add WSACleanup stuff properly. i probally have to clean up in other places too.
	WSACleanup();
}

Peer Peer::CreatePeer(bool server, unsigned short portNumber)
{
	Peer whatev;
	// eh maybe i'll make this a thing one day. The constructor is fine for now.
	return whatev;
}

void Peer::StartPeer()
{
	

	m_udpListener.Start();
}

void Peer::ShutdownPeer()
{
	WSACleanup();
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
Packet* Peer::UDPReceivePacket()
{
	if (m_currentPacket == nullptr)
		return nullptr;

	// if the udp listener has given us a real packet, we can give the user that.
	return m_currentPacket;
}

void Peer::UDPSend(Packet packet)
{
	//Packet newPacket;
	//m_udpListener.Send(newPacket);
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
