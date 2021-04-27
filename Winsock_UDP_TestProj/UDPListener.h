#pragma once
#include "WinsockHeader.h"
#include <string>


/// <summary>
/// Right now this UDPListener will only support IPv4.
/// </summary>
class UDPListener
{
public:
	UDPListener(std::string ipAddress, std::string portNumber);

	void Start();
	void Close();

	void Update();

	bool IsRunning();
private:
	SOCKET m_hostSocket;

	bool m_isRunning = false;


	fd_set m_master;
	fd_set m_readReady;


	//sockaddr_in servAddress;
	
};