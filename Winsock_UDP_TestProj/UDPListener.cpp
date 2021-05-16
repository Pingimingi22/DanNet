#include "UDPListener.h"
#include <iostream>

#include "Packet.h"

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"

#include "Peer.h"

#include "CorePackets.h"
#include <mutex>


UDPListener::UDPListener(Peer* attachedPeer, std::string portNumber, std::string ipAddress)
{
	m_attachedPeer = attachedPeer;

	sockaddr_in hostAddress;
	memset(&hostAddress, 0, sizeof(sockaddr_in));

	hostAddress.sin_family = AF_INET;

	// --------------- Whether they pass in a port or not kind of defines whether they want a client or a server. ------------------ //
	// If they pass a port in, that means they want their specific application to be listenining on a special "known" port. Which is what servers usually do.
	// However, if they don't set a port, it probably means they want to listen in general to any incoming messages, like what a client would do.
	// ----------------------------------------------------------------------------------------------------------------------------- //

	try
	{
		USHORT port = std::stoi(portNumber);
		hostAddress.sin_port = htons(std::stoi(portNumber.c_str()));
	}
	catch (...)
	{
		// this means the passed in port wasn't able to convert. For now we'll just make it so the socket picks an ephemeral port to listen on, thereby making it a client.
		// also we wont bother setting the port to a wildcard thing because I think if I just leave that part of the sockaddr_in struct empty, the kernel will automatically chose an emphemeral port.

		// ok i think i need to say 0 for the port to get an ephemeral port.
		hostAddress.sin_port = 0;
	}


	// ----------------------- If they've passed in an approriate IPv4 address, we try convert it. ----------------------- //
	// Also, they probably wont ever wont to pass in an IP address as that limits the socket to what it can listen to.
	// Usually you want the kernal to chose an appropriate IP address for the socket.
		
	int ipConversionResult = inet_pton(AF_INET, ipAddress.c_str(), &hostAddress.sin_addr.S_un.S_addr);                    // The reason we don't have an else statement initialising the variable is
	if (ipConversionResult == 0 || ipConversionResult == -1)															  // because inet_pton() has an out return param that does it for us.
	{
		std::cout << "UDPListener received an address that was not able to be converted." << std::endl;
		hostAddress.sin_addr.S_un.S_addr = INADDR_ANY;
	}


	// ---------------------- Creating the host socket. ---------------------- // 
	m_hostSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_hostSocket == INVALID_SOCKET)
	{
		std::cout << "Error on UDPListener socket()." << std::endl;
	}
	// ------------------------- Binding the socket. ------------------------- //
	int result = bind(m_hostSocket, (sockaddr*)&hostAddress, sizeof(hostAddress));
	if (result == -1)
	{
		std::cout << "Error on UDPListener bind()" << std::endl;
	}

	
	//memset(&m_tempClientSize, 0, sizeof(sockaddr)); =============================== NOTE THIS MAY HAVE BEEN IMPORTANT =============================== 

	
	// ------------------------ Zeroing out and setting up the FD sets. ------------------------ // 
	FD_ZERO(&m_master);
	FD_ZERO(&m_readReady);

	FD_SET(m_hostSocket, &m_master);
	// ----------------------------------------------------------------------------------------- //

	DisplaySettings();
}

void UDPListener::Start()
{
	m_isRunning = true;
}

void UDPListener::Close()
{
	m_isRunning = false;
	closesocket(m_hostSocket);
	std::cout << "Closed hostSocket on a UDPListener." << std::endl;
}

void UDPListener::Update()
{
	// Probably not the optimal place to have this since this is the "listener" but I think it'll work fine. maybe.
	// --------------------------------------- Sending reliable packets and laggy packets on timers --------------------------------------- // 
	m_attachedPeer->UpdateReliableSends();

	m_attachedPeer->UpdateLagSends();
	// ------------------------------------------------------------------------------------------------------------------------------------ //


	// Again probably not the best place but it seems to be working okay so far.
	// ----------------------------- Updating client time outs to check if they're still connected [SERVER SHOULD DO THIS]  ----------------------------- //
	m_attachedPeer->TimeoutUpdate();
	// -------------------------------------------------------------------------------------------------------------------------------------------------- //
	 
	
	// ----------------------------- Sending out alive packets to the server [ONLY CLIENT's SHOULD DO THIS] ----------------------------- //
	if (m_attachedPeer->GetId() != -1) // If we have a client ID I thiiink that we can be sure we're dealing with a client. This does seem a bit dodgy but it'll work.
	{
		m_attachedPeer->SendAlive();
	}
	// ---------------------------------------------------------------------------------------------------------------------------------- //


	m_readReady = m_master;
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	if (select(m_hostSocket, &m_readReady, NULL, NULL, &tv) == -1)
	{
		std::cerr << "select() error." << std::endl;
	}
	
	Packet* incomingPacket = new Packet();
	if (FD_ISSET(m_hostSocket, &m_readReady))
	{
		char recvBuffer[256];


		// temporary cache of incoming client address.
		sockaddr_in incomingClientAddress;
		int incomingClientSize;
		incomingClientSize = sizeof(sockaddr_in); 

		int result = recvfrom(m_hostSocket, &incomingPacket->m_allBytes[0], 256, 0, (sockaddr*)&incomingClientAddress, &incomingClientSize);
		if (result > 0)
		{
			// Now that we've added super secret internal header's to each packet, we have to remove them before we do anything else.
			// If the super secret header tells us it's a reliable packet, we send an ACK only if we can store the packet. This is so the user of the library can do what they want with the packet,
			// if we send an ACK without being able to store the packet, the user won't ever know the packet arrived but an ACK would have been sent anyway.
			// =============================== REMOVING SUPER SERCRET INTERNAL HEADERS =============================== //
			int incomingPriority;
			GUID incomingGuid;

			incomingPacket->InternalHeaderDeserialize(incomingPriority, incomingGuid);

			incomingPacket->m_priority = (PacketPriority)incomingPriority;

			if ((PacketPriority)incomingPriority == PacketPriority::RELIABLE_UDP) // we need to send an acknowledgement back to whoever sent us this.
			{
				if (m_attachedPeer->m_packetQueue.size() <= 1) // Only want to acknowledge packet's if we can actually store them.
				{
					Packet ackPacket = Packet(PacketPriority::UNRELIABLE_UDP, incomingGuid); // the acknowledgement doesn't have to be reliable because if the client doesn't receive it, 
																								  // it will ask for another one anyway.
					int ackPackIdentifier = (int)MessageIdentifier::RELIABLE_UDP_ACK;
					ackPacket.Serialize(ackPackIdentifier);
					char incomingIPString[15]; 
					inet_ntop(AF_INET, &incomingClientAddress.sin_addr.S_un.S_addr, &incomingIPString[0], 25);
					unsigned short incomingPort = ntohs(incomingClientAddress.sin_port);
					SendTo(ackPacket, incomingIPString, incomingPort);
				}
			}
			// ======================================================================================================= //


			

			switch (incomingPacket->GetPacketIdentifier())
			{
				// Hopefully only server's only ever send RELIABLE_UDP_ACK's. this way we can ensure we are dealing with the client in this case. --- Disregard this, client's can send reliable UDP too now.
			case MessageIdentifier::RELIABLE_UDP_ACK: // This is for when the client receives an ACK and wants to verify if the ack's sequence number matches any of it's reliable packet sequence numbers.
			{
				if (m_attachedPeer->m_packetQueue.size() <= 1)
				{
					bool guidMatches = true;
					for (int i = 0; i < m_attachedPeer->m_reliablePackets.size(); i++)
					{
						if (m_attachedPeer->m_reliablePackets[i].m_guid.Data1 == incomingGuid.Data1 &&
							m_attachedPeer->m_reliablePackets[i].m_guid.Data2 == incomingGuid.Data2 &&
							m_attachedPeer->m_reliablePackets[i].m_guid.Data3 == incomingGuid.Data3)
						{
							// Now to check the hexedecimal part of the guid.
							for (int j = 0; j < 8; j++)
							{
								if (m_attachedPeer->m_reliablePackets[i].m_guid.Data4[j] != incomingGuid.Data4[j])
								{
									guidMatches = false;
									break;
								}
							}
							if (guidMatches)
							{
								// The incoming guid matches one of the client's reliable packet guid's.

								// I guess we can clear the reliable packet from the client since we know the server received the message.
								int printIdentifier = (int)m_attachedPeer->m_reliablePackets[i].GetPacketIdentifier();
								std::cout << std::endl;
								std::cout << "======================================" << std::endl;
								std::cout << "Reliable UDP packet with identifier of " << printIdentifier << " was acknowledged and GUID of ." << m_attachedPeer->m_reliablePackets[i].m_guid.Data1 << ", " << m_attachedPeer->m_reliablePackets[i].m_guid.Data2 << ", " << m_attachedPeer->m_reliablePackets[i].m_guid.Data3;
								for (int x = 0; x < 8; x++)
								{
									std::cout << "," << m_attachedPeer->m_reliablePackets[i].m_guid.Data4[x];
								}
								std::cout << std::endl;
								std::cout << "======================================" << std::endl;


								// Removing it from the packet queue so user's don't have to deal with this type of packet.
								std::lock_guard<std::mutex> guard(*m_attachedPeer->m_reliablePacketMutex.get()); // Maybe we are reading in and erasing at the same time and that might be what is causing issues.
								m_attachedPeer->m_reliablePackets.erase(m_attachedPeer->m_reliablePackets.begin() + i); // guess this is why containers start at 0, so you can do this cool trick.
							}

						}
					}
					if (!guidMatches) // this means no guid matched the ack which is a pretty bad error and I don't think this should ever happen? Maybe if the server and client are really desynced?
					{
						std::cout << "Received a reliable UDP ACK that didn't match any known GUID!" << std::endl;
					}
				}
				break;
			}
			case MessageIdentifier::CONNECT:
				if (m_attachedPeer->m_packetQueue.size() <= 1)
				{
					std::cout << std::endl;
					std::cout << "=============================== NEW CLIENT ATTEMPTING TO CONNECT =============================== " << std::endl;
					m_attachedPeer->AddClient(incomingClientAddress);
				}
				break;
			case MessageIdentifier::ACK_CONNECT:
			{
				if (m_attachedPeer->m_packetQueue.size() <= 1)
				{
					ACKConnection AC;
					incomingPacket->Deserialize(AC.firstByte, AC.clientID, AC.port);
					m_attachedPeer->m_ID = AC.clientID;
					std::cout << "Server has acknowledged our connection. Our client ID is: " << m_attachedPeer->m_ID << "." << std::endl;
				}
				break;
			}
			case MessageIdentifier::CLIENT_ALIVE:
			{
				if (m_attachedPeer->m_packetQueue.size() <= 1)
				{
					ClientAlive clientAliveStruct;
					incomingPacket->Deserialize(clientAliveStruct.MessagIdentifier, clientAliveStruct.clientID);
					//std::cout << "Received an alive packet from one of the clients. Resetting client timeout..."<< std::endl;
					m_attachedPeer->GetClient(clientAliveStruct.clientID)->ResetTimer();
				}
				break;
			}
			default:
				if (m_attachedPeer->m_packetQueue.size() <= 1)
				{
					std::lock_guard<std::mutex> guard(*m_attachedPeer->m_packetMutex); // Incredibly important lock guard. Without it we can push while FlushCurrentPacket erases and that causes
					m_attachedPeer->m_packetQueue.push_back(incomingPacket);		   // an error that is sooo rare but crashes the entire program.
					MessageIdentifier testingIdentifier = incomingPacket->GetPacketIdentifier();
					std::cout << std::endl;
					std::cout << "Received a non core packet with an identifier of: " << (int)incomingPacket->GetPacketIdentifier() << std::endl;
					std::cout << std::endl;
				}
				break;
			}

		}
		else if (result == -1)
		{
			// ==================================== NOTE ======================================
			// Must delete incomingPacket here since it wasn't put into the vector which means the user has no means of freeing up the memory.
			// ================================================================================
			delete incomingPacket;
			m_attachedPeer->m_currentPacket = nullptr; // have to set that to nullptr so that way it flushes out the old packet.
			
		}
		else if (result == 0)
		{
			// ==================================== NOTE ======================================
			// Must delete incomingPacket here since it wasn't put into the vector which means the user has no means of freeing up the memory.
			// ================================================================================
			delete incomingPacket;
			m_attachedPeer->m_currentPacket = nullptr;
			
			//// i'm not exactly sure what 0 means but i do know that it means we havn't received any bytes.
		}
	}
	else
	{
		// if we havn't received anything.
		//m_attachedPeer->m_currentPacket = nullptr;

		delete incomingPacket;
		


		//std::cout << "UDPListener timeout." << std::endl;
	}

}

void UDPListener::Send(Packet& packet)
{
	int sendResult = sendto(m_hostSocket, packet.m_allBytes, 256, 0, (sockaddr*)&m_attachedPeer->m_serverConnection, sizeof(sockaddr_in));
	if (sendResult == -1)
	{
		std::cout << "An error occured when trying to send a message." << std::endl;
	}
	else if (sendResult > 0)
	{
		std::cout << "Successfully sent out [" << sendResult << "] bytes." << std::endl;
	}
}

void UDPListener::SendTo(Packet& packet, char* ipAddress, unsigned short port)
{
	sockaddr_in recipientAddress;
	recipientAddress.sin_family = AF_INET;
	inet_pton(AF_INET, &ipAddress[0], &recipientAddress.sin_addr.S_un.S_addr);
	recipientAddress.sin_port = htons(port);

	int sendResult = sendto(m_hostSocket, packet.m_allBytes, 256, 0, (sockaddr*)&recipientAddress, sizeof(sockaddr_in));

	if (sendResult == -1)
	{
		std::cout << "An error occured when trying to send the message." << std::endl;
	}
	else if (sendResult > 0)
	{
		std::cout << "Successfully sent message to. " << ipAddress << ", " << port << " Bytes sent [" << sendResult << "]." << std::endl;
	}
}

void UDPListener::DisplaySettings()
{
	sockaddr_in tempAddress;
	int tempAddressSize = sizeof(sockaddr_in);
	getsockname(m_hostSocket, (sockaddr*)&tempAddress, &tempAddressSize); // Getting the information from the host socket so that we can display it to the user.

	char thisIPString[256];
	inet_ntop(AF_INET, (sockaddr*)&tempAddress.sin_addr.S_un.S_addr, &thisIPString[0], 256);

	std::cout << "======================== SETTINGS ========================" << std::endl;
	std::cout << "IP Address: " << thisIPString << std::endl;
	std::cout << "Port Number: " << ntohs(tempAddress.sin_port) << std::endl;
	std::cout << "=================================================================" << std::endl;
}

bool UDPListener::IsRunning()
{
	if (m_isRunning)
		return true;
	return false;
}
