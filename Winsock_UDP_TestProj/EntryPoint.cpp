#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>

#include "UDPListener.h"

int main()
{
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