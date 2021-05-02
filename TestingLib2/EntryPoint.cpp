#include "Peer.h"
#include "Packet.h"


struct TestStruct
{
	int foo = 0;
	int bar = 0;

};
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
			
			TestStruct testStruct;
			testPacket->Deserialize(testStruct.foo, testStruct.bar);
			std::cout << testStruct.foo << std::endl;

			std::cout << "received message" << std::endl;
			isRunning = false;
		}
	}

	return 0;
}