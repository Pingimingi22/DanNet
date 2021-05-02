#include "Peer.h"
#include "Packet.h"

int main()
{
	Peer testPeer = Peer();

	testPeer.StartPeer();

	testPeer.Update();

	Packet* testPacket = testPeer.UDPReceivePacket();

	//testPeer.UDPSend()

	return 0;
}