#include "UDPListener.h"
#include <iostream>

#include "Packet.h"

UDPListener::UDPListener(std::string portNumber, std::string ipAddress)
{
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
		Packet incomingPacket = Packet();


		// temporary cache of incoming client address.
		sockaddr_in incomingClientAddress;
		int incomingClientSize;
		incomingClientSize = sizeof(sockaddr_in); 

		int result = recvfrom(m_hostSocket, &incomingPacket.m_allBytes[0], 1024, 0, (sockaddr*)&incomingClientAddress, &incomingClientSize);
		if (result > 0)
		{
			std::cout << "Received message." << std::endl;
			std::string receivedString = recvBuffer;
			
			incomingPacket.Write(1024);
			
			MessageIdentifier packetIdentifier = incomingPacket.GetPacketIdentifier();
			std::cout << "Message packet identifier: " << packetIdentifier << std::endl;
		}
		else if (result == -1)
		{
			std::cerr << "UDPListener recvfrom() error." << std::endl;

		}
	}
	else
	{
		//std::cout << "UDPListener timeout." << std::endl;
	}

}

void const UDPListener::Receive(const char* buffer)
{
	//return nullptr;
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
