#include "Peer.h"
#include "Packet.h"
#include "CorePackets.h"
void HandleInput()
{
	
}

struct TestStruct
{
	int foo = 0;
	int bar = 0;

};

enum test
{
	move_request = MessageIdentifier::CUSTOM_USER_ENUM,
	jump_request,
	trade_request

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
			//TestStruct testStruct;
			//testPacket->Deserialize(testStruct.foo, testStruct.bar);
			switch (testPacket->GetPacketIdentifier())
			{
			case MessageIdentifier::CONNECT:
				std::cout << "CONNECTION MESSAGE" << std::endl;
				testPeer.FlushCurrentPacket();
				break;
			case (MessageIdentifier)test::jump_request:
				std::cout << "JUMP REQUEST" << std::endl;
				testPeer.FlushCurrentPacket();
				break;
			case (MessageIdentifier)test::move_request:
				std::cout << "MOVE REQUEST" << std::endl;
				testPeer.FlushCurrentPacket();
				break;
			case (MessageIdentifier)test::trade_request:
				std::cout << "TRADE REQUEST" << std::endl;
				testPeer.FlushCurrentPacket();
				break;
			default:
				std::cout << "REGULAR MESSAGE" << std::endl;
				testPeer.FlushCurrentPacket();
				break;
			}

			//ConnectionStruct testStruct;
			//testPacket->Deserialize(testStruct.firstByte, testStruct.ip);
			//testPacket->GetPacketIdentifier();
			//std::cout << testStruct.firstByte << std::endl;
			//
			//std::cout << "received message" << std::endl;
			//isRunning = false;
		}
	}

	return 0;
}