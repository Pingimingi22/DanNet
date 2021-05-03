#pragma once

/// =================================== MessageIdentifiers.h =================================== ///
// This class is inspired by RakNet's way of handling packet types. RakNet makes the first byte of every
// packet an identifier of the type of message being sent. In this header file there will be some enums that
// users of the library can use and assign identifiers to.
/// ============================================================================================ ///

 
enum class MessageIdentifier
{
	CONNECT = 0, // client sends connect.
	ACK_CONNECT, // server sends acknowledgement.
	DISCONNECT = 1,
	ECHO = 2,
	REQUEST_GREETING = 3,
	CUSTOM_USER_ENUM = 4
};

