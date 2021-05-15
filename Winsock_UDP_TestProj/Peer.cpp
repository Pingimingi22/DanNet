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

	m_reliablePacketMutex = std::make_unique<std::mutex>();

	m_lagPacketMutex = std::make_unique<std::mutex>();


	// testing reserving spaces for our std::vector's since I'm having memory issues.
	m_reliablePackets.reserve(MAX_RELIABLE_PACKET_QUEUE_SIZE);
	if (server) // we will only have connected client's if we are the server, so no point reserving space if we are the client.
	{
		m_connectedClients.reserve(10);
	}
	m_packetQueue.reserve(10);

	m_lagPacketQueue.reserve(MAX_LAG_PACKET_QUEUE_SIZE);


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

	Packet* connectionPacket = new Packet((int)PacketPriority::UNRELIABLE_UDP);
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


	//m_udpListener.Send(*connectionPacket);
	UDPSend(*connectionPacket);
	//delete connectionPacket;

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
		std::lock_guard<std::mutex> guard(*m_reliablePacketMutex);
		m_reliablePackets.push_back(packet);
		std::cout << "Added reliable packet to reliable packet queue." << std::endl;
	}

	// We only want to add packets to the lag queue if they are:
	// - Not reliable.
	// - The peer is set to simulate lag.
	else if (m_isLagSimulation && m_lagPacketQueue.size() < MAX_LAG_PACKET_QUEUE_SIZE)
	{
		std::lock_guard<std::mutex> lagPacketGaurd(*m_lagPacketMutex.get());
		m_lagPacketQueue.push_back(packet);
		std::cout << "Added a packet to the lag packet queue." << std::endl;
	}


	// If the peer is attempting to simulate lag, we don't want to immediately send, we want to make sure it's as slow as possible.
	if (!m_isLagSimulation)
	{
		m_udpListener.Send(packet);
	}
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
	// ======================== Setting the transmission rate ======================== //
	// Although we have already set a transmission rate, we need to know which one to use.
	// If we are trying to simulate lag, we'll use the requested lag in millesconds for the transmission rate.
	double transmissionRate;
	if (m_isLagSimulation)
		transmissionRate = m_lagInMilliseconds;
	else
		transmissionRate = RELIABLE_UDP_RETRANSMISSION_RATE;
	// =============================================================================== //

	if (m_reliablePackets.size() > 0)
	{
		for (int i = 0; i < m_reliablePackets.size(); i++)
		{
			if(!m_reliablePackets[i].m_isTimerStarted)
				m_reliablePackets[i].StartPacketTimer();

			m_reliablePackets[i].CheckPacketTimer();
			m_reliablePackets[i].GetTimeDuration();

			if (m_reliablePackets[i].m_elapsedMilliseconds >= m_lagInMilliseconds) 
			{
				MessageIdentifier type = m_reliablePackets[i].GetPacketIdentifier();
				
				if (!m_isServer) // if we're not the server we're probably connected so we can use Send()
				{
					m_udpListener.Send(m_reliablePackets[i]);																		 // ================================== IMPORTANT NOTE ================================== // 
					std::cout << "Sent out a reliable udp packet with Send() again of type [" << (int)type << "]." << std::endl;	 // The reason why I'm using m_udpListener.Send() instead of UDPSend() is because UDPSend()
					m_reliablePackets[i].StopPacketTimer();																		 // will add the packet to the packet queue, and since we are "re-sending" packets, we would
				}																													 // keep duplicating packet's if we used UDPSend().
				
				else // otherwise this is the server and we have to use the SendTo() function.
				{
					m_udpListener.SendTo(m_reliablePackets[i], m_reliablePackets[i].m_destinationIP, m_reliablePackets[i].m_destinationPort);														
					std::cout << "Sent out a reliable udp packet with SendTo() again of type [" << (int)type << "]." << std::endl;	 
					m_reliablePackets[i].StopPacketTimer();
				}
				


			}																										 
		}
	}
}

void const Peer::UDPSendTo(Packet& packet, char* ipAddress, unsigned short port)
{
	if (packet.m_priority == PacketPriority::RELIABLE_UDP)
	{
		if (m_isServer) // If we are the server let's cache the destination ip address and port into this packet so we can continously send it in the reliable udp update function.
		{
			//strcpy_s(packet.m_destinationIP, ipAddress);
			//packet.m_destinationPort = port;
			packet.SetDestination(ipAddress, port);
		}

		std::lock_guard<std::mutex> guard(*m_reliablePacketMutex);
		m_reliablePackets.push_back(packet);
		std::cout << "Added reliable packet to reliable packet queue." << std::endl;
	}

	// If the packet is unreliable and we want to simulate lag, we'll put it in the lag packet queue.
	else if (m_isLagSimulation && m_lagPacketQueue.size() < MAX_LAG_PACKET_QUEUE_SIZE)
	{
		packet.SetDestination(ipAddress, port); // Setting the ip and port of the packet so it knows where to go.

		std::lock_guard<std::mutex> lagPacketGuard(*m_lagPacketMutex.get());
		m_lagPacketQueue.push_back(packet);
		std::cout << "Added a packet to the lag packet queue." << std::endl;
	}
	if(!m_isLagSimulation) // If we aren't simulating lag, send out a packet instantly without waiting for reliable UDP re-transmission.
	{
		m_udpListener.SendTo(packet, ipAddress, port);
	}
}

void const Peer::UDPSendToAll(Packet& packet)
{
	// there is an issue when sending a packet to everyone and that is you are sending 1 packet with 1 guid to everyone. if anyone client ack's the packet, you clear it from the reliable packet queue.
	// really each packet should have a unique GUID which is what I'm going to try do here.

	int priority;
	GUID originalGUID;
	
	packet.InternalHeaderDeserialize(priority, originalGUID);
	
	// but of course we only want a unique GUID if they are sending a reliable UDP packet.
	if (priority == int(PacketPriority::RELIABLE_UDP))
	{
		
	
		for (int i = 0; i < m_connectedClients.size(); i++)
		{
			Packet uniquePacket(priority);
			// to get the binary data from the original packet into this one, i'm gonna try memcpy.
			memcpy(&uniquePacket.m_allBytes[0], &packet.m_allBytes[0], 256);

			//memcpy(&uniquePacket.m_destinationIP[0], &packet.m_destinationIP[0], 15);
			//uniquePacket.m_destinationPort = packet.m_destinationPort;

			uniquePacket.SetDestination(packet.m_destinationIP, packet.m_destinationPort);

			uniquePacket.m_guid.Data1 = packet.m_guid.Data1;
			uniquePacket.m_guid.Data2 = packet.m_guid.Data2;
			uniquePacket.m_guid.Data3 = packet.m_guid.Data3;

			for (int x = 0; x < 8; x++)
			{
				uniquePacket.m_guid.Data4[x] = packet.m_guid.Data4[x];
			}

			UDPSendTo(uniquePacket, m_connectedClients[i].m_ipAddress, m_connectedClients[i].m_port);
		}
	
	}

	// just send the normal packet if they don't care about reliability.
	else
	{
	
		for (int i = 0; i < m_connectedClients.size(); i++)
		{

			// ----------- Making a unique packet with it's own unique GUID ----------- //
			// Techanically we don't need to send unique packet's since GUID's wont even be checked if it's not a reliable packet.
			// ------------------------------------------------------------------------ //

			Packet uniquePacket(priority);
			// to get the binary data from the original packet into this one, i'm gonna try memcpy.
			memcpy(&uniquePacket.m_allBytes[0], &packet.m_allBytes[0], 256);

			//memcpy(&uniquePacket.m_destinationIP[0], &packet.m_destinationIP[0], 15);
			//uniquePacket.m_destinationPort = packet.m_destinationPort;

			uniquePacket.SetDestination(packet.m_destinationIP, packet.m_destinationPort);

			uniquePacket.m_guid.Data1 = packet.m_guid.Data1;
			uniquePacket.m_guid.Data2 = packet.m_guid.Data2;
			uniquePacket.m_guid.Data3 = packet.m_guid.Data3;

			for (int x = 0; x < 8; x++)
			{
				uniquePacket.m_guid.Data4[x] = packet.m_guid.Data4[x];
			}

			// ------------------------------------------------------------------------ //

			UDPSendTo(packet, m_connectedClients[i].m_ipAddress, m_connectedClients[i].m_port);
		}
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

void Peer::SimulateLag(bool isSimulate, double lagInMilliseconds)
{
	// There are two core things to consider when attempting to simulate lag. We need to put all packets into some sort of queue and time their "release". But there are two types of packets, reliable and
	// unreliable udp packets. Reliable packets don't get stored anywhere and we can just throw them into a lag queue and send them out slowly, but UDP packets get sent out initially like unreliable packets,
	// but are then put into their own reliable UDP queue. The reliable udp queue has it's owner timer stuff going on to time the release of the re-transmission of packets.

	// Because reliable UDP packet's are already in a queue of their own which gets released on a timer, to simulate lag we will only put unreliable packets into the lag queue and let reliable packet's go into
	// their reliable udp queue. To make reliable packets "lag", I'm going to set the re-transmission rate of reliable udp packets to match the simulated lag time passed in.

	m_isLagSimulation = isSimulate;
	m_lagInMilliseconds = lagInMilliseconds;
}

void Peer::UpdateLagSends()
{

	// =========================== NOTE =========================== //
	// Because simulated lag packet's will never be reliable,
	// we can clear them from the m_lagPacketQueue as soon as we
	// send them out.
	// ============================================================ //


	if (m_lagPacketQueue.size() > 0)										
	{
		//std::lock_guard<std::mutex> lagPacketGuard(*m_lagPacketMutex.get());		//  ==================== IMPORTANT ==================== //						// ==== ANOTHER IMPORTANT NOTE ==== //
		for (int i = 0; i < m_lagPacketQueue.size(); i++)							// This lock_guard HAS to be here, otherwise, when								// I'm keeping the old note here for
		{																			// another thread writes to the lagPacketQueue, if the vector					// historic reasons but I've changed
			if (!m_lagPacketQueue[i].m_isTimerStarted)								// isn't big enough, the vector will do a bunch of dynamic						// how the system works so we no longer
				m_lagPacketQueue[i].StartPacketTimer();								// allocation and moving around which messes up the vector if					// have to lock before the for loop.
																					// writing occurs at the same time.												// We are resrving enough space to not
			m_lagPacketQueue[i].CheckPacketTimer();																													// have to worry about writing/iterating
			m_lagPacketQueue[i].GetTimeDuration();																													// at the same time.


			if (m_lagPacketQueue[i].m_elapsedMilliseconds >= m_lagInMilliseconds)
			{
				MessageIdentifier type = m_lagPacketQueue[i].GetPacketIdentifier();

				if (!m_isServer) // if we're not the server we're probably connected so we can use Send()
				{
					std::lock_guard<std::mutex> lagPacketGuard(*m_lagPacketMutex);
					m_udpListener.Send(m_lagPacketQueue[i]);																		 // ================================== IMPORTANT NOTE ================================== // 
					std::cout << "Sent out a laggy udp packet with Send() of type [" << (int)type << "]." << std::endl;				 // The reason why I'm using m_udpListener.Send() instead of UDPSend() is because UDPSend()
					m_lagPacketQueue[i].StopPacketTimer();																		     // will add the packet to the packet queue, and since we are "re-sending" packets, we would
																																	 // keep duplicating packet's if we used UDPSend().																			

					// Removing packet from the queue since we've sent it out an no longer need to track it.
					m_lagPacketQueue.erase(m_lagPacketQueue.begin() + i); // Removing the packet that just sent.
					i--;
				}																													 

				else // otherwise this is the server and we have to use the SendTo() function.
				{
					std::lock_guard<std::mutex> lagPacketGuard(*m_lagPacketMutex.get());
					m_udpListener.SendTo(m_lagPacketQueue[i], m_lagPacketQueue[i].m_destinationIP, m_lagPacketQueue[i].m_destinationPort);
					std::cout << "Sent out a laggy udp packet with SendTo() of type [" << (int)type << "]." << std::endl;
					m_lagPacketQueue[i].StopPacketTimer();

					// Removing packet from the queue since we've sent it out an no longer need to track it.
					m_lagPacketQueue.erase(m_lagPacketQueue.begin() + i); // Removing the packet that just sent.
					i--;
				}

			}
		}
	}
}

void Peer::Update()
{
	while (m_udpListener.IsRunning())
	{
		//std::cout << "Update test" << std::endl;
		m_udpListener.Update();
	}

	if (!m_udpListener.IsRunning())
	{
		std::cout << "hey" << std::endl;
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

	std::cout << std::endl;
	std::cout << "==== Client connected! Client IP: " << client.m_ipAddress << " Port: " << client.m_port << " ID No. " << client.m_clientID << "." << "====" << std::endl;

	// Letting the client know that we've accepted their connection.
	ACKConnection AC;
	//AC.firstByte = (int)MessageIdentifier::ACK_CONNECT;
	AC.clientID = client.m_clientID;
	AC.port = client.m_port;
	Packet ACPacket((int)PacketPriority::UNRELIABLE_UDP);
	ACPacket.Serialize(AC.firstByte, AC.clientID, AC.port);
	std::cout << "Sending connection acknowledgement to client." << std::endl;
	UDPSendTo(ACPacket, client.m_ipAddress, client.m_port);

	std::cout << std::endl;
}

void Peer::RemoveClient(char* ipAddress)
{
	// TODO do this function.
}
