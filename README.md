# DanNet
DanNet, a simple networking library that UDP sockets. It was created for my Complex Games Unit at AIE. It features reliable UDP and fake latency.


========================================== DanNet Manual ==========================================

This manual is a brief explanation of how to use DanNet. For more indepth examples see my 
implementation program which has an example of a graphical client and a server.


DanNet is a networking library that uses a single UDP socket to send and receive packets. User's
of DanNet can create their own packets that can contain a maximum size of 256 bytes. DanNet depends on
the c++ serialization library Cereal.

DanNet's Features:
	- Reliable UDP packets.
	- Lag simulation. (Stalls the time of which packets are sent out.)
	
	
	
To begin using DanNet add the Winsock_UDP_TestProj.lib and DanNet header files to your project.
For debug mode add the Winsock_UDP_TestProj-d.lib file instead. When you have added the library 
and header files, include the "DanNet.h" header to use DanNet in your project. 
	

========== Creating a Peer ==========
Peer's can either be in server mode or client mode depending on what you pass in the parameters.
By default, the parameters are set to false and 0 respectively, which would be used to create a client peer.
	
	Peer(bool server = false, unsigned short portNumber = 0)

	------- Code Example -------

	{
		Peer examplePeer(true, 25565); // Making a server peer.
		examplePeer.StartPeer();       // Call StartPeer() to fire up it's UDPListener. This will allow us to start sending and receiving.
	}
	
	
	You can choose to simulate lag by calling SimulateLag(bool isSimulate, double lagInMilliseconds = 0) after you have started your Peer.
	
=====================================



========== Making custom message identifiers ==========
An important part of DanNet is that the first thing user's should serialize into packets is what type of message the packet is.
To do that, you want to serialize a MessageIdentifier before you serialize anything else into the packet. To create your own
custom message identifiers, define an enum class named whatever you want and initialize the first enum to MessageIdentifier::CUSTOM_USER_ENUM.

	------- Code Example -------
	
	{
		enum ExampleCustomIdentifiers
		{
			EXAMPLE_ENUM1 = MessageIDentifier::CUSTOM_USER_ENUM,
			EXAMPLE_ENUM2   // We don't have to initialize this one as it will automatically be 1 higher than the previous.
		}
	}

=======================================================



========== Creating and sending packets ==========
When creating Packet's, depending on what you pass into the constructor, you can make either unreliable or reliable UDP packets.

	Packet examplePacket(PacketPriority priority)
	
After creating a packet, you need to put data into it. To do that, use the Serialize() function. Pass in any primitive type to Serialize()
and it will store it into the packet. The order of what you serialize matters as you will have to deserialize in the same order on the
receiving side.

An important rule to follow is that the first thing you should serialize into a packet is the MessageIdentifier. If you don't do this,
errors will occur within the internal code of DanNet.

	------- Code Example -------
	
	{
		CustomIdentifier messageType = EXAMPLE_ENUM1;
		Packet examplePacket(PacketPriority::UNRELIABLE)
		
		examplePacket.Serialize(int(messageType));
	}
	
	
To send packets you can use three different functions depending on your circumstances. 
You have access to UDPSend(Packet& packet), UDPSendTo(Packet& packet, char* ipAddress, unsigned short port) and UDPSendToAll(Packet& packet).

	- UDPSend() Should be used for clients wanting to send messages to the server. It requires that you have called Connect() on your peer and have
				successfully connected.
	
	- UDPSendTo() In the implementation, I've found UDPSendTo() to be useful for sending messages from the server to specific clients. It doesn't require
				  a peer that is connected and instead you pass in the ip address and port you want to send your message to.
	
	- UDPSendToAll() Should be used for the server when you want to send a message to every connected client.	
	
==================================================



========== Receiving packets ==========
To receive packets, you want to process every packet you currently have in your main game loop. An example can be found in the implementation but a summarised example
is below.

One of the most critical things you do in processing packets is to call FlushCurrentPacket() on your Peer after processing a packet. This tells DanNet to remove it
from the receive packet queue and will allow you to access the next packet.

Another part of processing packets is deserializing their data so you can use it. To deserialise you call the Deserialize() function on the packet.
To deserialize you need to pass in references to types where you're going to contain the data.

Deserialize() has a parameter pack for it's arguments so you can pass in as many things as you want, but remember to always deserialize the message type first and then
deserialize in the same order you serialized on the other end.

	------- Code Example -------
	
	{
		Packet* incomingPacket = nullptr;
		
		while(gameRunning)
		{
			incomingPacket = examplePeer.UDPReceivePacket();
			if(incomingPacket != nullptr)
			{
				switch(incomingPacket->GetPacketIdentifier())
				{
					case (MessageIdentifier)CustomIdentifier::EXAMPLE_ENUM1:
					
						// Do what you want with the packet but remember to flush it after you're done!
						
						examplePeer.FlushCurrentPacket();
						break;
				}
			}
		}
	}

=======================================
