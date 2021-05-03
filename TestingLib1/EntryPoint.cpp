#include "Peer.h"
#include "Packet.h"

#include "MessageIdentifiers.h"

struct TestStruct
{
	int foo = 1;
	int bar = 2;
};

enum test
{
	move_request = MessageIdentifier::CUSTOM_USER_ENUM,
	jump_request,
	trade_request


};

int main()
{
	Peer testPeer = Peer();

	testPeer.StartPeer();

	bool isRunning = true;

	testPeer.Connect("192.168.1.124", 25565);
	
	//Packet testPacket;
	//TestStruct testStruct;
	//testPacket.Serialize(testStruct.foo, testStruct.bar);
	//testPeer.UDPSend(testPacket);

	while (isRunning)
	{

	}


	Packet testPacket;
	TestStruct test;
	testPacket.Serialize(test.foo, test.bar);
	testPeer.UDPSend(testPacket);

	Packet jumpPacket;
	char firstByte = (int)test::jump_request;
	jumpPacket.Serialize(firstByte);
	testPeer.UDPSend(jumpPacket);

	Packet movePacket;
	char firstByte1 = (int)test::move_request;
	movePacket.Serialize(firstByte1);
	testPeer.UDPSend(movePacket);
	
	//Packet tradePacket;
	//char firstByte2 = (int)test::trade_request;
	//tradePacket.Serialize(firstByte2);
	//testPeer.UDPSend(tradePacket);
	

	


	return 0;
}