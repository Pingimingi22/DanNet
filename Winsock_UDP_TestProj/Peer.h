#pragma once

#include <string>
#include "UDPListener.h"
#include <sstream>
#include <thread>

#include <vector>
#include "ClientStruct.h"

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

	// These two Send() functions are to be used after a client is connected to a server. They require that a connection has been established.
	void const UDPSend(Packet& packet);
	void UDPSendReliable(Packet packet);

	// This Send() function can be used to send things to specific clients if you are the server.
	void const UDPSendTo(Packet& packet, char* ipAddress, unsigned short port);
	void const UDPSendToReliable(Packet& packet, char* ipAddress, unsigned short port);

	void const UDPSendToAll(Packet& packet);
	void UDPSendReliableToAll();

	void FlushCurrentPacket();


private:
	void Update();
	//Peer CreatePeer(bool server = false, unsigned short portNumber = NULL); // i guess this is gonna be a factory method.


	// Only to be used if the peer is the server.
	void const AddClient(sockaddr_in& clientAddress);
	void RemoveClient(char* ipAddress);

	bool m_isServer = false;
	UDPListener m_udpListener;
	SOCKET m_hostSocket;

	// Only to be set if the peer is a client.
	sockaddr_in m_serverConnection;


	Packet* m_currentPacket = nullptr;

	// should be empty if we're not the server.
	std::vector<ClientStruct> m_connectedClients;

	int m_clientCount = 0; // client's ID's will just be the clientCount of when they joined. So the first client that joins will have an ID of '0'.


	// ------------------ ONLY TO BE USED IF PEER IS A CLIENT. ------------------ //
	int m_ID = -1; // -1 is like an error checking thing.


	// -------------------- Threading stuff -------------------- //
	// ========================================== ASK FINN ========================================== why does putting this on the stack delete some assignment operator implicitly?
	std::thread* m_udpListenerUpdateThread = nullptr;
};