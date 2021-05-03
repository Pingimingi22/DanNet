#pragma once

#include "MessageIdentifiers.h"

/// The rules of a "Dan" packet are:
// Each packet's first byte must be a MessageIdentifier.
// The rest is the pay load.
// The client must know how to use the payload data, so this will involve making a struct server side aswell as client side.

// These "core" packet structs are used in things like establishing connections to and from the server with UDP and stuff.

struct ConnectionStruct
{
	char firstByte = (char)MessageIdentifier::CONNECT;
	char ip[256];
};