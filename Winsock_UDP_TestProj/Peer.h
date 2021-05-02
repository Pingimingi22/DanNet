#pragma once

#include <string>
#include "UDPListener.h"
#include <sstream>


class UDPListener;
class Packet;

class Peer
{
public:
	friend class UDPListener;
	Peer(bool server = false, unsigned short portNumber = NULL);
	Peer CreatePeer(bool server = false, unsigned short portNumber = NULL); // i guess this is gonna be a factory method.
	void StartPeer();
	void ShutdownPeer();

	void Connect(std::string ipAddress, unsigned short portNumber);

	Packet* UDPReceivePacket();

	void UDPSend(Packet packet);
	void UDPSendReliable(Packet packet);


	void Update();

private:
	bool m_isServer = false;
	UDPListener m_udpListener;
	SOCKET m_hostSocket;

	// Only to be set if the peer is a client.
	sockaddr_in m_serverConnection;


	Packet* m_currentPacket = nullptr;
};