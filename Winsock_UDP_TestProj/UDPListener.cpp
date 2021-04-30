#include "UDPListener.h"
#include <iostream>

UDPListener::UDPListener(std::string ipAddress, std::string portNumber)
{
	sockaddr_in servAddress;
	memset(&servAddress, 0, sizeof(sockaddr_in));

	memset(&m_tempClientSize, 0, sizeof(sockaddr));

	servAddress.sin_family = AF_INET;
	servAddress.sin_port = htons(std::stoi(portNumber.c_str()));
    //servAddress.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//servAddress.sin_addr.S_un.S_addr;
	//inet_pton(AF_INET, "10.17.33.83", (sockaddr*)&servAddress.sin_addr.S_un.S_addr);

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

	sockaddr_in testServerAddress;
	int testServerAddressSize = sizeof(sockaddr_in);
	getsockname(m_hostSocket, (sockaddr*)&testServerAddress, &testServerAddressSize);


	char testServerIP[256];
	inet_ntop(AF_INET, (sockaddr*)&testServerAddress, &testServerIP[0], 256);

	std::cout << "======================== SERVER SETTINGS ========================" << std::endl;
	std::cout << "IP Address: " << testServerIP << std::endl;
	std::cout << "Port Number: " << ntohs(testServerAddress.sin_port) << std::endl;
	std::cout << "=================================================================" << std::endl;
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


		m_tempClientSize = sizeof(sockaddr_in); 
		int result = recvfrom(m_hostSocket, &recvBuffer[0], 256, 0, (sockaddr*)&m_tempClient, &m_tempClientSize);
		if (result > 0)
		{
			std::cout << "Received message: " << recvBuffer << std::endl;
			std::string receivedString = recvBuffer;
			if (receivedString == "connection_request")
			{
				std::cout << "client connection request received." << std::endl;
				SOCKET newClientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
				if (newClientSocket == -1)
				{
					std::cout << "An error occured when trying to create a socket for a connecting client." << std::endl;
				}
				if (bind(newClientSocket, (sockaddr*)&m_tempClient, m_tempClientSize) == -1)
				{
					std::cout << "An error occured when trying to bind the newly created client." << std::endl;
				}
				else
				{
					short port = ntohs(m_tempClient.sin_port);
					m_connectedClients.push_back(newClientSocket);
				}

				
			}
			if (receivedString == "request_hello")
			{
				std::cout << "client connection requested a hello." << std::endl;
				for (int i = 0; i < m_connectedClients.size(); i++)
				{

					sockaddr_in tempAddress;
					int tempSize = sizeof(sockaddr_in);
					getsockname(m_connectedClients[i], (sockaddr*)&tempAddress, &tempSize);
					if (tempAddress.sin_addr.S_un.S_addr == m_tempClient.sin_addr.S_un.S_addr)
					{
						// found the connected client to return a message to.
						char helloMessage[256] = "hello!";
						if (sendto(m_hostSocket, &helloMessage[0], 256, 0, (sockaddr*)&tempAddress, tempSize) == -1)
						{
							std::cout << "Error occured when trying to send a hello message to the client." << std::endl;
							
							break;
						}
						else
						{
							char testIP[256];
							short testPort = tempAddress.sin_port;
							inet_ntop(AF_INET, (sockaddr*)&tempAddress.sin_addr.S_un.S_addr, &testIP[0], 256);
							testPort = ntohs(tempAddress.sin_port);
							std::cout << "Client IP: " << testIP << " : " << "Port: " << htons(tempAddress.sin_port) << std::endl;
							std::cout << "Sent client a hello." << std::endl;
						}
						break;
					}

					
				}


			}
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

void const UDPListener::Send(const char* buffer)
{
	// hardcoding buffer length to 256, this will be changed to allow for dynamic sized things to be sent.
	//sendto(m_hostSocket, buffer, 256, 0, (sockaddr*))
}

void const UDPListener::Receive(const char* buffer)
{
	//return nullptr;
}

bool UDPListener::IsRunning()
{
	if (m_isRunning)
		return true;
	return false;
}
