#include "UDPListener.h"
#include <iostream>

#include "Packet.h"

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"

#include "Peer.h"

#include "CorePackets.h"
#include <mutex>

// DELETE THIS LATER.
struct TestStruct
{
	int hello = 0;
	int goodbye = 0;
	int test1 = 0;
	int test2 = 0;
	int test3 = 0;
};

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
	
	m_readReady = m_master;
	timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	if (select(m_hostSocket, &m_readReady, NULL, NULL, &tv) == -1)
	{
		std::cerr << "select() error." << std::endl;
	}
	
	if (FD_ISSET(m_hostSocket, &m_readReady))
	{
		char recvBuffer[256];
		Packet* incomingPacket = new Packet();


		// temporary cache of incoming client address.
		sockaddr_in incomingClientAddress;
		int incomingClientSize;
		incomingClientSize = sizeof(sockaddr_in); 

		int result = recvfrom(m_hostSocket, &incomingPacket->m_allBytes[0], 256, 0, (sockaddr*)&incomingClientAddress, &incomingClientSize);
		if (result > 0)
		{
			//std::cout << "Received message." << std::endl;
			//std::string receivedString = recvBuffer;
			//
			//std::stringstream ss;
			//ss.write(&incomingPacket.m_allBytes[0], 1024);
			//cereal::BinaryInputArchive iarchive(ss);
			//
			//TestStruct testReadingIn;
			//iarchive(testReadingIn.hello, testReadingIn.goodbye);
			//
			//int hi = 5;
			//float byte = 3;
			//char alpha = 'a';
			//int idk1 = 2;
			//int idk2 = 1; 
			//float fin = 0;
			//incomingPacket.Deserialize(hi, byte, alpha, idk1, idk2, fin);
			
			//std::cout << "Received a message." << std::endl;
			//
			//TestStruct testingReadingIn;
			//incomingPacket->Deserialize(testingReadingIn.hello, testingReadingIn.goodbye, testingReadingIn.test1, testingReadingIn.test2, testingReadingIn.test3); // mwhahaha my multiple param any type verdaic function ! >:)
			//std::cout << testingReadingIn.goodbye;

			// We check every incoming packet's first byte. If they are sending a CorePacket we deal with it here so the user doesn't have to.
			switch (incomingPacket->GetPacketIdentifier())
			{
			case MessageIdentifier::CONNECT:
				m_attachedPeer->AddClient(incomingClientAddress);
				return;
				break;
			case MessageIdentifier::ACK_CONNECT:
			{
				ACKConnection AC;
				incomingPacket->Deserialize(AC.firstByte, AC.clientID, AC.port);
				m_attachedPeer->m_ID = AC.clientID;
				std::cout << "Server has acknowledged our connection. Our client ID is: " << m_attachedPeer->m_ID << "." << std::endl;
			}
			default:
				break;
			}
			

			if (m_attachedPeer->m_currentPacket != nullptr)
			{
				std::lock_guard<std::mutex> guard(*m_attachedPeer->m_packetMutex);
				delete m_attachedPeer->m_currentPacket;
				m_attachedPeer->m_currentPacket = nullptr;                // This is the only place I'm freeing up the memory of m_currentPacket. So there wont be that bad of a memory leak since every time we receive
			}															  // a new packet, it will delete the old one.

			m_attachedPeer->m_currentPacket = incomingPacket; // ------------------------> Telling the attached peer that we have received a packet. The user can do what they like with it.
			//incomingPacket.Write(1024);
			//
			//MessageIdentifier packetIdentifier = incomingPacket.GetPacketIdentifier();
			//std::cout << "Message packet identifier: " << packetIdentifier << std::endl;
		}
		else if (result == -1)
		{
			delete incomingPacket;
			m_attachedPeer->m_currentPacket = nullptr; // have to set that to nullptr so that way it flushes out the old packet.

			std::cerr << "UDPListener recvfrom() error." << std::endl;

		}
		else if (result == 0)
		{
			delete incomingPacket;
			m_attachedPeer->m_currentPacket = nullptr;

			// i'm not exactly sure what 0 means but i do know that it means we havn't received any bytes.
		}
	}
	else
	{
		// if we havn't received anything.
		//m_attachedPeer->m_currentPacket = nullptr;


		//std::cout << "UDPListener timeout." << std::endl;
	}

}

void const UDPListener::Receive(const char* buffer)
{
	//return nullptr;
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

//void UDPListener::SendReliable(Packet packet)
//{
//}

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
		std::cout << "Successfully sent message. Bytes sent [" << sendResult << "]." << std::endl;
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
