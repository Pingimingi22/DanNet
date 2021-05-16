#pragma once

#include "MessageIdentifiers.h"

/// The rules of a "Dan" packet are:
// As far as user's of DanNet are converned, each packet's first four bytes must be a MessageIdentifier.
// The rest is the pay load. Internally there is another header that becomes before the first four bytes the user sets but this is dealt by the library.
// The client must know how to use the payload data, so this will involve making a struct server side aswell as client side.

// These "core" packet structs are used in things like establishing connections to and from the server with UDP and stuff.


struct ConnectionStruct
{
	int firstByte = (int)MessageIdentifier::CONNECT;
	char ip[25];
	// Changed ip from 256 bytes to 25 bytes. 256 bytes overloads our entire packet and causes issues with Cereal serialization.
};

struct ACKConnection
{
	int firstByte = (int)MessageIdentifier::ACK_CONNECT;
	int clientID = -1;
	unsigned short port = -1;
};

struct ACKUDP
{
	int firstByte = (int)MessageIdentifier::RELIABLE_UDP_ACK;
};

// Client's will continuously send this to their server or whatever they're connected to to keep their connection. If the server doesn't receive this within a certain time frame, the server will
// send out a ClientTimeout to all other remaining clients which will signal them to delete their reference to the timed out client.
struct ClientAlive
{
	int MessagIdentifier = (int)MessageIdentifier::CLIENT_ALIVE;
	int clientID;
};

// Client's will receive this from the main server and can use it for whatever suits their program.
struct ClientTimeout
{
	int MessageIdentifier = (int)MessageIdentifier::CLIENT_TIMEOUT;
	int clientID;
};