#pragma once
#include "WinsockHeader.h"
#include <string>
#include <vector>



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

	void const Send(const char* buffer);
	void const Receive(const char* buffer);


	bool IsRunning();
private:
	SOCKET m_hostSocket;

	std::vector<SOCKET> m_connectedClients;

	bool m_isRunning = false;


	fd_set m_master;
	fd_set m_writeReady;
	fd_set m_readReady;



	// temporary cache of incoming client address. FIXME should these be located here?
	sockaddr_in m_tempClient;
	int m_tempClientSize;
	
	
	
};