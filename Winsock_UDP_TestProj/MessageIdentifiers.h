#pragma once

/// =================================== MessageIdentifiers.h =================================== ///
// This class is inspired by RakNet's way of handling packet types. RakNet makes the first byte of every
// packet an identifier of the type of message being sent. In this header file there will be some enums that
// users of the library can use and assign identifiers to.
/// ============================================================================================ ///

 
enum class MessageIdentifier
{
	CONNECT = 9, // client sends connect.
	ACK_CONNECT = 1, // server sends acknowledgement.
	DISCONNECT = 2,
	ECHO = 3,
	REQUEST_GREETING = 4,
	RELIABLE_UDP_ACK = 5,
	CUSTOM_USER_ENUM = 6
};

