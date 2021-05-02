#include "Peer.h"
#include "Packet.h"

struct TestStruct
{
	int foo = 1;
	int bar = 2;
};

int main()
{
	Peer testPeer = Peer();

	testPeer.StartPeer();

	bool isRunning = true;

	testPeer.Connect("192.168.1.124", 25565);

	Packet testPacket;
	TestStruct testStruct;
	testPacket.Serialize(testStruct.foo, testStruct.bar);

	testPeer.UDPSend(testPacket);

	


	return 0;
}