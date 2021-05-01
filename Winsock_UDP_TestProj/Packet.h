#pragma once

#include <sstream>

#include "MessageIdentifiers.h"

class Packet
{
public:
	Packet();
	void Create();
	void Send();
	void SendReliable();

	void Write(int howManyBytes);

	MessageIdentifier GetPacketIdentifier(); // Only to be used after one byte has been read from the packet.

	/// <summary>
	/// Serialize() serializes a struct into the m_allBytes char array.
	/// </summary>
	/// <param name="structToSerialize">Pass in the struct you want to serialize.</param>
	void Serialize(void* structToSerialize);


private:
	static constexpr int maxPacketSize = 1024;



public:
	// These probably shouldn't be public but it's easier this way to read things into these byte arrays all the way in the UDPListener.

	char m_allBytes[maxPacketSize]; // idk 1024 bytes (1KB) seemed like a cool number to pick for the maximum amount in a "dan" packet.
				      // I think this is super low for today's standards but eh maybe it'll be cool having an old school networking library.
	std::stringstream m_readBytes[maxPacketSize];

};

