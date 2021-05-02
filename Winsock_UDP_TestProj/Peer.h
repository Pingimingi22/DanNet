#pragma once

#include <string>
#include "UDPListener.h"
#include <sstream>
#include <thread>

class UDPListener;
class Packet;

class Peer
{
public:
	friend class UDPListener;
	Peer(bool server = false, unsigned short portNumber = NULL);
	~Peer();

	void StartPeer();
	void ShutdownPeer();

	void Connect(std::string ipAddress, unsigned short portNumber);

	Packet* UDPReceivePacket();

	void UDPSend(Packet packet);
	void UDPSendReliable(Packet packet);



private:
	void Update();
	//Peer CreatePeer(bool server = false, unsigned short portNumber = NULL); // i guess this is gonna be a factory method.


	bool m_isServer = false;
	UDPListener m_udpListener;
	SOCKET m_hostSocket;

	// Only to be set if the peer is a client.
	sockaddr_in m_serverConnection;


	Packet* m_currentPacket = nullptr;


	// -------------------- Threading stuff -------------------- //
	// ========================================== ASK FINN ========================================== why does putting this on the stack delete some assignment operator implicitly?
	std::thread* m_udpListenerUpdateThread = nullptr;
};