#include "TemporaryHeader.h"

#include <thread>
#include <string>

void HandleInput(std::string messageBuffer, sockaddr* serverAddress) // ================================= ASK WHY SOCKADDR DOESN'T WORK HERE BUT IT DOES EVERYWHERE ELSE.
{
	while (true)
	{
		std::string testBuffer;
		std::getline(std::cin, testBuffer);
		int sendResult = sendto(m_hostSocket, &testBuffer[0], 256, 0, (sockaddr*)serverAddress, sizeof(sockaddr_in));

		sockaddr_in hostSocketAddress;
		int hostSocketAddressSize = sizeof(sockaddr_in);
		getsockname(m_hostSocket, (sockaddr*)&hostSocketAddress, &hostSocketAddressSize);

		std::cout << "Port Number: " << ntohs(hostSocketAddress.sin_port) << std::endl;
		char testIP[256];
		inet_ntop(AF_INET, (sockaddr*)&hostSocketAddress.sin_addr.S_un.S_addr, &testIP[0], 256);
		std::cout << "IP: " << ntohl(hostSocketAddress.sin_addr.S_un.S_addr) << std::endl;
		if (sendResult == -1)
		{
			std::cout << "An error occured when trying to send a message." << std::endl;
		}
	}
}


int main()
{


	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);




	sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(sockaddr_in));

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(25565);


	inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr.S_un.S_addr);

	m_hostSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//sockaddr_in testHostAddress


	char message[256];

	fd_set readable;
	fd_set writable;
	fd_set master_set;


	FD_ZERO(&readable);
	FD_ZERO(&writable);
	FD_ZERO(&master_set);

	FD_SET(m_hostSocket, &master_set);

	int serverSize = sizeof(sockaddr_in);


	

	sockaddr_in hostSocketAddress;
	hostSocketAddress.sin_port = 0; // trying to get a ephemeral port.
	hostSocketAddress.sin_family = AF_INET;
	hostSocketAddress.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // trying to get a wild card ip address.

	bind(m_hostSocket, (sockaddr*)&hostSocketAddress, sizeof(sockaddr_in));
	int hostSocketSize = sizeof(sockaddr_in);
	getsockname(m_hostSocket, (sockaddr*)&hostSocketAddress, &hostSocketSize);

	char ipPrintable[256];
	if (inet_ntop(AF_INET, (sockaddr*)&hostSocketAddress.sin_addr.S_un.S_addr, &ipPrintable[0], 256) == NULL)
	{
		std::cout << "error occured with inet_ntop()" << std::endl;
	}
	std::cout << "======================== CLIENT SETTINGS ========================" << std::endl;
	std::cout << "IP Address: " << ipPrintable << std::endl;
	std::cout << "Port Number: " << ntohs(hostSocketAddress.sin_port) << std::endl;
	std::cout << "=================================================================" << std::endl;

	


	std::string messageBuffer;
	std::thread inputHandler = std::thread(HandleInput, messageBuffer, (sockaddr*)&serverAddress);

	//sockaddr_in testAddress;
	//int testAddressSize;
	//getsockname(m_hostSocket, (sockaddr*)&sockaddr_in, )

	//std::cout << "HOST PORT: " << 
	

	timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	char recvBuffer[256] = "hellllo?";
	std::string inputBuffer;
	while (true)
	{
		readable = master_set;
		writable = master_set;
		if (select(m_hostSocket, &readable, NULL, NULL, &tv) == -1) // =============== Currently using select as a way to poll both sending and receiving functions. =============== //
		{
			std::cout << "An error occured with select()." << std::endl;
		}

		if (FD_ISSET(m_hostSocket, &readable))
		{
			// we have stuff to read.
			char recvBuffer[256];
			
			serverSize = sizeof(sockaddr_in);
			int recvResult = recvfrom(m_hostSocket, &recvBuffer[0], 256, 0, (sockaddr*)&serverAddress, &serverSize);
			if(recvResult > 0)
			{
				std::cout << "server says: " << recvBuffer << std::endl;
			}
			else if (recvResult == -1)
			{
				std::cout << "error occured in recvfrom()." << std::endl;
			}

		}
		//if (FD_ISSET(m_hostSocket, &writable))
		//{
		//	
		//	// we have stuff to write.
		//	char sendBuffer[256];
		//	std::cin >> sendBuffer;
		//	if (sendto(m_hostSocket, &sendBuffer[0], 256, 0, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
		//	{
		//		std::cerr << "error occured in sendto()" << std::endl;
		//	}
		//}
		else
		{
			//std::cout << "Reading and writing is not ready." << std::endl;
		}
		
	}

	

	



	WSACleanup();

	system("pause");

	return 0;
}