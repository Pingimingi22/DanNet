#pragma once

#include "MessageIdentifiers.h"

/// The rules of a "Dan" packet are:
// Each packet's first byte must be a MessageIdentifier.
// The rest is the pay load.
// The client must know how to use the payload data, so this will involve making a struct server side aswell as client side.

// These "core" packet structs are used in things like establishing connections to and from the server with UDP and stuff.

struct ConnectionStruct
{
	int firstByte = /*(int)MessageIdentifier::CONNECT*/ 21;
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