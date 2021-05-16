#pragma once

/// =================================== MessageIdentifiers.h =================================== ///
// This class is inspired by RakNet's way of handling packet types. RakNet makes the first byte of every
// packet an identifier of the type of message being sent. In this header file there will be some enums that
// users of the library can use and assign identifiers to.
/// ============================================================================================ ///

 
enum class MessageIdentifier
{
	CONNECT = 0, // client sends connect.
	ACK_CONNECT = 1, // server sends acknowledgement.
	DISCONNECT = 2,
	ECHO = 3, // never implemented.
	REQUEST_GREETING = 4, // removed, used for super early testing stuff.
	RELIABLE_UDP_ACK = 5,
	CLIENT_TIMEOUT = 6,
	CLIENT_ALIVE = 7,
	SERVER_TIMEOUT = 8, // currently not implemented, only clients can timeout.
	SERVER_ALIVE = 9, // currently not implemented.
	CUSTOM_USER_ENUM = 10
};

