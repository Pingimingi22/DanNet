#include "UDPListener.h"
#include <iostream>

UDPListener::UDPListener(std::string ipAddress, std::string portNumber)
{
	sockaddr_in servAddress;
	memset(&servAddress, 0, sizeof(sockaddr_in));

	servAddress.sin_family = AF_INET;
	servAddress.sin_port = htons(std::stoi(portNumber.c_str()));
	servAddress.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	m_hostSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_hostSocket == INVALID_SOCKET)
	{
		std::cout << "Error on UDPListener socket()." << std::endl;
	}
	
	int result = bind(m_hostSocket, (sockaddr*)&servAddress, sizeof(servAddress));
	if (result == -1)
	{
		std::cout << "Error on UDPListener bind()" << std::endl;
	}

	FD_ZERO(&m_master);
	FD_ZERO(&m_readReady);

	FD_SET(m_hostSocket, &m_master);
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
		int result = recvfrom(m_hostSocket, &recvBuffer[0], 256, 0, NULL, NULL);
		if (result > 0)
		{
			std::cout << "Received message: " << recvBuffer << std::endl;
		}
		else if (result == -1)
		{
			std::cerr << "UDPListener recvfrom() error." << std::endl;
		}
	}
	else
	{
		std::cout << "UDPListener timeout." << std::endl;
	}

}

bool UDPListener::IsRunning()
{
	if (m_isRunning)
		return true;
	return false;
}
