#include "Peer.h"
//#include "UDPListener.h"

#include <string>

#include "Packet.h"

#include <assert.h>

#include <thread>

#include "CorePackets.h"

Peer::Peer(bool server, unsigned short portNumber)
{
	WSADATA wsadata;
	int result = WSAStartup(MAKEWORD(2, 2), &wsadata);

	// initialising my test mutex.
	m_packetMutex = std::make_unique<std::mutex>();


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
	ShutdownPeer();
	WSACleanup();
}

//Peer Peer::CreatePeer(bool server, unsigned short portNumber)
//{
//	Peer whatev;
//	// eh maybe i'll make this a thing one day. The constructor is fine for now.
//	return whatev;
//}

void Peer::StartPeer()
{
	m_udpListener.Start();

	m_udpListenerUpdateThread = new std::thread(&Peer::Update, this);

	//UDPListenerThread.join();
}

void Peer::ShutdownPeer()
{
	m_udpListener.Close();
	m_udpListenerUpdateThread->join();
	delete m_udpListenerUpdateThread;
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

	m_serverConnection = serverAddress;
	std::cout << "Attempting to connection to " << ipAddress << " on port: " << portNumber << std::endl;

	// ============ TODO ============= MAKE IT SO THE SERVER GETS A CONNECTION TO THE CLIENT.

	// ------------------------------- Sending the server a Connection packet which contains our IP ------------------------------- //

	sockaddr_in hostAddress;
	int hostSize = sizeof(sockaddr_in);

	getsockname(m_hostSocket, (sockaddr*)&hostAddress, &hostSize);

	Packet* connectionPacket = new Packet((int)PacketPriority::RELIABLE_UDP);
	ConnectionStruct connection;
	//connection.ip = ntohl(hostAddress.sin_addr.S_un.S_addr);
	inet_ntop(AF_INET, &hostAddress.sin_addr.S_un.S_addr, &connection.ip[0], 25); // idk 256 is just random.
	
	connectionPacket->Serialize(connection.firstByte, connection.ip);

	// delete this just for testing.
	int testPacketPriority;
	GUID testGuid;
	connectionPacket->InternalHeaderDeserialize(testPacketPriority, testGuid);

	ConnectionStruct testingConnection;
	testingConnection.firstByte = 0;
	connectionPacket->Deserialize(testingConnection.firstByte, testingConnection.ip);
	std::cout << "=================================" << std::endl;
	std::cout << testingConnection.firstByte << std::endl;
	std::cout << "=================================" << std::endl;
	// --------------------------


	m_udpListener.Send(*connectionPacket);
	delete connectionPacket;

}

/// <summary>
/// ReceivePacket() will return null if no packet has been received. Otherwise it returns a packet.
/// </summary>
/// <returns></returns>
Packet* Peer::UDPReceivePacket()
{
	//if (m_currentPacket == nullptr)
	//	return nullptr;
	//
	//// if the udp listener has given us a real packet, we can give the user that.
	//return m_currentPacket;

	if (m_packetQueue.size() == 0)
	{
		return nullptr;
	}

	return m_packetQueue[0];
}

void const Peer::UDPSend(Packet& packet)
{
	//Packet newPacket;


	// With our new system of packet priority headers, we need to "seceretly" apply these headers to each packet. We do that here in the Send() functions.
	// This send function will apply a header that specifies the message is not reliable and will set the integer in the header to be -1.

	if (packet.m_priority == PacketPriority::RELIABLE_UDP)
	{
		m_reliablePackets.push_back(&packet);
		std::cout << " Added reliable packet to reliable packet queue." << std::endl;
	}

	
	m_udpListener.Send(packet);
}

void Peer::UDPSendReliable(Packet& packet)
{
	// TODO actually make this thing.
	//m_udpListener.send
	//std::thread reliableSendThread = std::thread
	//while()
	//m_reliablePackets.push_back(&packet);
	std::cout << "Added a packet to the reliable send queue." << std::endl;
}

void Peer::UpdateReliableSends()
{
	if (m_reliablePackets.size() > 0)
	{
		UDPSend(*m_reliablePackets[0]);
	}
}

void const Peer::UDPSendTo(Packet& packet, char* ipAddress, unsigned short port)
{
	if (packet.m_priority == PacketPriority::RELIABLE_UDP)
	{
		m_reliablePackets.push_back(&packet);
		std::cout << " Added reliable packet to reliable packet queue." << std::endl;
	}

	m_udpListener.SendTo(packet, ipAddress, port);
}

void const Peer::UDPSendToAll(Packet& packet)
{
	for (int i = 0; i < m_connectedClients.size(); i++)
	{
		UDPSendTo(packet, m_connectedClients[i].m_ipAddress, m_connectedClients[i].m_port);
	}
}

/// <summary>
/// FlushCurrentPacket() is so that user's can clear the m_currentPacker variable after they've done what they wanted to do with it, (I'm not sure if I've managed memory correctly.)
/// </summary>
void Peer::FlushCurrentPacket()
{
	//std::lock_guard<std::mutex> guard(*m_packetMutex);
	//if(m_currentPacket != nullptr)
	//	delete m_currentPacket;
	//m_currentPacket = nullptr;

	if (m_packetQueue[0] != nullptr)
	{
		delete m_packetQueue[0];
		m_packetQueue.erase(m_packetQueue.begin());
	}

}

ClientStruct Peer::GetClient(int id)
{
	for (int i = 0; i < m_connectedClients.size(); i++)
	{
		if (m_connectedClients[i].m_clientID == id)
		{
			return m_connectedClients[i];
		}
	}
	// otherwise they tried to find a client that doesn't exist.
	assert(true);
}

void Peer::Update()
{
	while (m_udpListener.IsRunning())
	{
		m_udpListener.Update();
	}
}

void const Peer::AddClient(sockaddr_in& clientAddress)
{
	ClientStruct client;
	inet_ntop(AF_INET, &clientAddress.sin_addr.S_un.S_addr, &client.m_ipAddress[0], 25);
	client.m_clientID = m_clientCount;
	client.m_port = ntohs(clientAddress.sin_port);

	m_clientCount++; // increasing the connected clients counter.

	m_connectedClients.push_back(client);

	std::cout << "Client connected! Client IP: " << client.m_ipAddress << " Port: " << client.m_port << " ID No. " << client.m_clientID << "." << std::endl;

	// Letting the client know that we've accepted their connection.
	ACKConnection AC;
	//AC.firstByte = (int)MessageIdentifier::ACK_CONNECT;
	AC.clientID = client.m_clientID;
	AC.port = client.m_port;
	Packet ACPacket((int)PacketPriority::RELIABLE_UDP);
	ACPacket.Serialize(AC.firstByte, AC.clientID, AC.port);
	UDPSendTo(ACPacket, client.m_ipAddress, client.m_port);

	std::cout << "Sending connection acknowledgement to client." << std::endl;
}

void Peer::RemoveClient(char* ipAddress)
{
	// TODO do this function.
}
