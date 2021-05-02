#pragma once
#include "WinsockHeader.h"
#include <string>
#include <vector>

class Peer;
class Packet;

/// <summary>
/// Right now this UDPListener will only support IPv4.
/// </summary>
class UDPListener
{

public:
	//friend class Peer;
	UDPListener() {};
	UDPListener(Peer* attachedPeer, std::string portNumber = "", std::string ipAddress = "");

	void Start();
	void Close();

	void Update();

	void const Receive(const char* buffer);

	void Send(Packet packet);
	void SendReliable(Packet packet);

	void DisplaySettings();


	bool IsRunning();
private:
	SOCKET m_hostSocket; // points to the socket in Peer.h

	std::vector<SOCKET> m_connectedPeers; // This is connected "peer's" because this listener could be listenting to lots of clients if it's the server or it could be listenting to 1 peer (the server).
									      // the idea is that this UDPListener class will be able to be used for both clients and server's.

	bool m_isRunning = false;


	fd_set m_master;
	fd_set m_writeReady;
	fd_set m_readReady;

	Peer* m_attachedPeer;
	
};