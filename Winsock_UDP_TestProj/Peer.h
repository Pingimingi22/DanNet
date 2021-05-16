#pragma once

#include <string>
#include "UDPListener.h"
#include <sstream>
#include <thread>

#include <vector>
#include "Client.h"

#include <mutex>

#define RELIABLE_UDP_RETRANSMISSION_RATE 2000 
#define MAX_LAG_PACKET_QUEUE_SIZE 100				// ================================== IMPORTANT ===================================== //
#define MAX_RELIABLE_PACKET_QUEUE_SIZE 100			// These two defines are important because of having to reserve space for the packet  //
													// vectors. If we don't reserve space, dynamic allocation will kick in and ruin       //
class UDPListener;									// everything due to multithreading issues with writing/iterating at the same time.   //
class Packet;										// ================================================================================== //
													
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
	// This Send() function can be used to send things to specific clients if you are the server.
	void const UDPSendTo(Packet& packet, char* ipAddress, unsigned short port);
	void const UDPSendToAll(Packet& packet);

	void UpdateReliableSends();



	// Clears the packet first in the packet queue.
	void FlushCurrentPacket();

	int GetId() { return m_ID; }
	// Returns a ClientStruct containing the ip address and port of the client.
	Client* GetClient(int id);


	void SimulateLag(bool isSimulate, double lagInMilliseconds = 0);
	void UpdateLagSends();
	
	


private:
	void Update();


	// Only to be used if the peer is the server.
	void const AddClient(sockaddr_in& clientAddress);
	void RemoveClient(char* ipAddress);

	bool m_isServer = false;
	UDPListener m_udpListener;
	SOCKET m_hostSocket;

	// Only to be set if the peer is a client.
	sockaddr_in m_serverConnection;

	std::unique_ptr<std::mutex> m_packetMutex;
	Packet* m_currentPacket = nullptr;
	std::vector<Packet*> m_packetQueue;

	// This is a container of the packet's we want to make sure get sent. We only remove them from this container after we receive an acknowledgement from the server.
	std::unique_ptr<std::mutex> m_reliablePacketMutex;
	std::vector<Packet> m_reliablePackets;


	// should be empty if we're not the server.
	std::vector<Client> m_connectedClients;

	int m_clientCount = 0; // client's ID's will just be the clientCount of when they joined. So the first client that joins will have an ID of '0'.


	// ------------------ ONLY TO BE USED IF PEER IS A CLIENT. ------------------ //
	int m_ID = -1; // -1 is like an error checking thing.



	// ----------------- Lag simulation stuff. ----------------- //
	std::unique_ptr<std::mutex> m_lagPacketMutex;
	std::vector<Packet> m_lagPacketQueue;
	double m_lagInMilliseconds = 0;
	bool m_isLagSimulation = false;



	// ----------------- Time out stuff. -------------------- //
	// Will iterate through all client's and will check how long since they sent a client alive function.
	// I guess this should only be used for the server.
	void TimeoutUpdate();

	// To prevent us from sending like a million alive packets per frame, I'll make a timer that sends them out every 1.5 seconds or so. Maybe I'll make this time adjustable.
	void SendAlive();
	double m_aliveSendOutTime = 1500;
	bool m_readyToSendAlive = true;
	bool m_hasAliveSendTimerStarted = false;
	std::chrono::time_point<std::chrono::system_clock> m_aliveSendStart;
	std::chrono::time_point<std::chrono::system_clock> m_aliveSendEnd;

	// Mutex for m_connectedClients std::vector since we iterate and erase on two seperate threads.
	std::unique_ptr<std::mutex> m_connectedClientsMutex;
	




	// -------------------- Threading stuff -------------------- //
	// ========================================== ASK FINN ========================================== why does putting this on the stack delete some assignment operator implicitly?
	std::thread* m_udpListenerUpdateThread = nullptr;
};