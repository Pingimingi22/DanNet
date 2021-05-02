#include "Peer.h"
#include "Packet.h"

int main()
{
	Peer testPeer = Peer(true, 25565);

	testPeer.StartPeer();

	bool isRunning = true;
	while (isRunning)
	{
		Packet* testPacket = testPeer.UDPReceivePacket();
		if (testPacket != nullptr)
		{
			std::cout << "received message" << std::endl;
			isRunning = false;
		}
	}

	return 0;
}