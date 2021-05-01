#include "TemporaryHeader.h"

#include <thread>
#include <string>

void HandleInput(std::string messageBuffer, sockaddr* serverAddress) // ================================= ASK WHY SOCKADDR DOESN'T WORK HERE BUT IT DOES EVERYWHERE ELSE.
{
	while (true)
	{
		std::string testBuffer;
	    std::getline(std::cin, testBuffer);

		int sendResult = 0;

		if(testBuffer.size() > 0) // making sure the buffer is greater than zero so we don't accidentally send nothing.
			sendResult = sendto(m_hostSocket, &testBuffer[0], 256, 0, (sockaddr*)serverAddress, sizeof(sockaddr_in));

		sockaddr_in hostSocketAddress;
		int hostSocketAddressSize = sizeof(sockaddr_in);
		getsockname(m_hostSocket, (sockaddr*)&hostSocketAddress, &hostSocketAddressSize);

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



	// -------------------------------- Setting up server address -------------------------------- //
	sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(sockaddr_in));

	serverAddress.sin_family = AF_INET;
	// ------------------------------------------------------------------------------------------- //


	// -------------------------------- Startup Info -------------------------------- //
	DisplayStartupInfo(serverAddress.sin_addr.S_un.S_addr, serverAddress.sin_port);
	// ----------------------------------------------------------------------------- //

	


	// ---------------------------- Setting up host socket ---------------------------- //
	sockaddr_in hostSocketAddress;
	hostSocketAddress.sin_port = 0; // trying to get a ephemeral port.
	hostSocketAddress.sin_family = AF_INET;
	m_hostSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	bind(m_hostSocket, (sockaddr*)&hostSocketAddress, sizeof(sockaddr_in));
	// -------------------------------------------------------------------------------- //

	int serverSize = sizeof(sockaddr_in);


	// ---------------------------- Setting up file descriptor sets ---------------------------- //
	fd_set readable;
	fd_set writable;
	fd_set master_set;


	FD_ZERO(&readable);
	FD_ZERO(&writable);
	FD_ZERO(&master_set);

	FD_SET(m_hostSocket, &master_set);
	// ----------------------------------------------------------------------------------------- //


	DisplaySocketAddress(m_hostSocket);
	

	// testing sending something
	unsigned int testByte = 5;
	sendto(m_hostSocket, (char*)&testByte, sizeof(testByte), 0, (sockaddr*)&serverAddress, sizeof(sockaddr_in));


	// --------------------------- Creating new thread for input handling --------------------------- //
	std::cout << "Input thread start." << std::endl;
	std::string messageBuffer;
	std::thread inputHandler = std::thread(HandleInput, messageBuffer, (sockaddr*)&serverAddress);
	// ---------------------------------------------------------------------------------------------- //

	

	timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	std::string inputBuffer;
	while (true)
	{
		readable = master_set;
		writable = master_set;
		if (select(m_hostSocket, &readable, NULL, NULL, &tv) == -1) // =============== Currently using select as a way to poll both sending and receiving functions. =============== //
		{
			std::cout << "An error occured with select()." << std::endl;
			int error = WSAGetLastError();
			std::cout << "Error code: " << error << std::endl;
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
				int errorcode = WSAGetLastError();
			}

		}
		
		else
		{
			//std::cout << "Reading and writing is not ready." << std::endl;
		}
		
	}

	WSACleanup();

	system("pause");

	return 0;
}

void DisplayStartupInfo(ULONG& address, USHORT& port)
{
	char input[256];

	bool isInputRunning = true;

	std::cout << "================================== CLIENT ==================================" << std::endl;
	while (isInputRunning)
	{
		std::cout << "ENTER SERVER IP: ";
		std::cin >> input;
		if (inet_pton(AF_INET, &input[0], &address) == 0) // address is an out param and we are returning the typed in IP address into it.
		{
			std::cout << "Error occured with inet_pton in DisplayStartupInfo()" << std::endl;
			continue; // asking them to type it in again incase they entered something incorrectly.
		}

		std::cout << "ENTER SERVER PORT: ";
		std::cin >> input;
		int inputNumerical;
		try
		{
			inputNumerical = std::stoi(input);
		}
		catch (...)
		{
			continue; // asking them to redo the input stuff because they entered something incorrectly.
		}
		port = htons(inputNumerical); // assigning the typed in port but in network endianness to the passed in port parameter.
		std::cout << "============================================================================" << std::endl;
		std::cout << "Setting successfully entered." << std::endl;
		isInputRunning = false;
	}


}

void DisplaySocketAddress(SOCKET socket)
{
	sockaddr_in socketAddress;
	int addressSize = sizeof(sockaddr_in);
	getsockname(socket, (sockaddr*)&socketAddress, &addressSize);


	char ipPrintable[256];

	if (inet_ntop(AF_INET, &socketAddress.sin_addr.S_un.S_addr, &ipPrintable[0], 256) == NULL)
	{
		std::cout << "Error occured on inet_ntop in DisplaySocketAddress()." << std::endl;
	}
	std::cout << "======================== CLIENT SETTINGS ========================" << std::endl;
	std::cout << "IP Address: " << ipPrintable << std::endl;
	std::cout << "Port Number: " << ntohs(socketAddress.sin_port) << std::endl;
	std::cout << "=================================================================" << std::endl;
}