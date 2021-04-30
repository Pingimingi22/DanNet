#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>

#include "UDPListener.h"

#include <thread>
#include <string>

void HandleInput(std::string inputBuffer)
{
	bool inputIsRunning = true;

	while (inputIsRunning)
	{
		std::getline(std::cin, inputBuffer);

		if (inputBuffer == "exit")
		{
			std::cout << "Closing program.";
			return;
		}

		std::cout << inputBuffer << std::endl;
		//sendto()
	}
}


int main()
{
	// ------------- Testing make a new thread. ------------- //
	std::string inputBuffer;
	std::thread testThread = std::thread(HandleInput, inputBuffer);

	WSADATA wsadata;
	int result = WSAStartup(MAKEWORD(2, 2), &wsadata);
	
	if (result != 0)
	{
		std::cout << "Error occured on WSAStartup()." << std::endl;
	}

	UDPListener testListener = UDPListener("*", "25565");
	testListener.Start();
	while (testListener.IsRunning())
	{
		testListener.Update();
	}

	WSACleanup();

	return 0;

}