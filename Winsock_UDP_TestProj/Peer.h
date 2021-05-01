#pragma once

#include <string>
#include "UDPListener.h"


class UDPListener;
class Packet;

class Peer
{
public:
	Peer();
	Peer CreatePeer(bool server = false, unsigned short portNumber = NULL); // i guess this is gonna be a factory method.
	void StartPeer();
	void ShutdownPeer();

	Packet GetCurrentPacket();

	void Update();

private:
	bool m_isServer = false;
	UDPListener m_udpListener;
};