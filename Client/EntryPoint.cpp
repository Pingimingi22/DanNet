#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <stdio.h>
#include <string>

int main()
{

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);


	SOCKET clientSocket;
	//SOCKET servSocket;

	sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(sockaddr_in));

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(25565);


	inet_pton(AF_INET, "192.168.1.124", &serverAddress.sin_addr.S_un.S_addr);

	clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	char message[256];

	fd_set readable;
	fd_set master_set;


	FD_ZERO(&readable);
	master_set = readable;


	int serverSize;

	timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	char recvBuffer[256] = "hellllo?";
	std::string inputBuffer;
	while (true)
	{
		getline(std::cin, inputBuffer);

		if (sendto(clientSocket, &inputBuffer[0], 256, 0, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
		{
			std::cerr << "sendto() error" << std::endl;
		}
	}

	

	



	WSACleanup();

	system("pause");

	return 0;
}