#pragma once

#include <sstream>

#include "MessageIdentifiers.h"

#include <iostream>

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"

#include "PacketPriorities.h"

//#include <stdint.h>

#include "combaseapi.h"


class Packet
{
	// Making peer a friend so that it can access SerializeInternal and DeserializeInternal.
	friend class Peer;
	friend class UDPListener;

	int32_t something;

public:
	Packet(PacketPriority priority);

private:
	Packet() {} // We need a default constructor for Packet's because the UDPListener needs to be able to create a "generic" packet that it will fill in when it receives data.
	Packet(PacketPriority priority, GUID guid); // Special constructor only to be used internally. NOT by the user. When the udp listener needs to send an ACK back, 
		                                        //they will construct the packet with this constructor so they can hand pick the GUID.

public:
	void Create();
	void Send();
	void SendReliable();


	void Clear();
	

	PacketPriority GetPacketPriority();
	
	MessageIdentifier GetPacketIdentifier(); // Only to be used after one byte has been read from the packet.

	PacketPriority m_priority;
	GUID m_guid;


	// ----------------------------- Derserialize verdaic function unpacking technique in progress here! ----------------------------- //
	// Unfortunately function parameter pack's cannot be iterated through and we have to do a recursive technique that involves having a base case (the first Deserialize() function and a recursive
	// case the second Deserialize() function).

	// --------> Helpful resource to understand this: https://kevinushey.github.io/blog/2016/01/27/introduction-to-c++-variadic-templates/

	std::stringstream m_recursiveStream; // make sure to clear this after use.

	template<typename T>
	void Deserialize(T& t)
	{
		//std::stringstream ss; // ------------> Unfortunately we can't create streams inside these functions because the base case and the resursive case functions need to use the same stream. :(
		//m_recursiveStream.write(&m_allBytes[0], 1024);
		cereal::BinaryInputArchive iarchive(m_recursiveStream);
		iarchive(t);
		//std::cout << 1 << std::endl;
		m_recursiveStream.clear();	
	}

	template<typename T, typename... Args>
	void Deserialize(T& first, Args& ... args)
	{
		//std::stringstream ss;										// ============================================================ NOTE ================================================================== //
		//m_recursiveStream.write(&m_allBytes[0], 256);				// kind of confused why this is even working. Realistically, I shouldn't be rewriting everything to the string stream each type this    //
																	// recursive function gets called. I think the Cereal library is saving me with it's really good input/output serialization functions.  //
																	// ==================================================================================================================================== //
		cereal::BinaryInputArchive iarchive(m_recursiveStream); 
		iarchive(first);
		

		Deserialize(args...);
	}
	// ------------------------------------------------------------------------------------------------------------------------------- //

	// --------------------------------------- More veriadic functions here. This time it's serializing. --------------------------------------- //
 	
	template<typename T>
	void Serialize(T& t)
	{
		cereal::BinaryOutputArchive outputArchive(m_recursiveStream);
		outputArchive(t);

		m_recursiveStream.read(&m_allBytes[0], 256);
		m_recursiveStream.clear();
	}
	
	/// <summary>
	/// Serialize() serializes a struct into the m_allBytes char array.
	/// </summary>
	/// <param name="structToSerialize">Pass in the struct you want to serialize.</param>
	template<typename T, typename... Args>
	void Serialize(T& first, Args& ... args)
	{
		cereal::BinaryOutputArchive outputArchive(m_recursiveStream);

		outputArchive(first);

		Serialize(args...);
	}

	// ----------------------------------------------------------------------------------------------------------------------------------------- //
private:

	void InternalHeaderSerialize(PacketPriority Priority, GUID* guid = nullptr)
	{
		// I need to quickly serialize the header data before serializing the user's payload data.
		// But I only know what to put in the header data after the user has already serialized their stuff.

		cereal::BinaryOutputArchive outputArchive(m_recursiveStream);

		GUID testGuid;
		memset(&testGuid, 0, sizeof(GUID));


		

		// Serializing the packet priority.
		outputArchive((int)Priority);
		// Serializing the unique GUID. We only need to generate a GUID if the parsed in priority is reliable udp.
		if (Priority == PacketPriority::RELIABLE_UDP && guid == nullptr) // This means they don't want a specific GUID, so we'll create a new one for them.
		{
			CoCreateGuid(&testGuid);
			memcpy(&m_guid, &testGuid, sizeof(testGuid)); // giving the packet a cache of the guid so we can use it for easy comparisons in the UDPListener.
			// Not sure if memcpy is safe but whatev lol
		}
		else if (Priority == PacketPriority::RELIABLE_UDP && guid != nullptr) // This means the parsed in a GUID so we'll set this GUID to theirs. It's probably a server sending an ACK.
		{
			memcpy(&testGuid, &guid, sizeof(guid)); // putting the stuff into our cache of testGuid.
			memcpy(&m_guid, &testGuid, sizeof(testGuid)); // now putting it into the packet's cache.

		}

		outputArchive(testGuid.Data1);
		outputArchive(testGuid.Data2);
		outputArchive(testGuid.Data3);
		outputArchive(testGuid.Data4);

	}
	void InternalHeaderDeserialize(PacketPriority& priority, GUID& guid)
	{

		m_recursiveStream.write(&m_allBytes[0], 256);

		cereal::BinaryInputArchive inputArchive(m_recursiveStream);

		inputArchive(priority);

		inputArchive(guid.Data1);
		inputArchive(guid.Data2);
		inputArchive(guid.Data3);
		inputArchive(guid.Data4);
	}


	static constexpr int maxPacketSize = 256;

	void Write(int howManyBytes);

public:
	// These probably shouldn't be public but it's easier this way to read things into these byte arrays all the way in the UDPListener.

	char m_allBytes[maxPacketSize]; // idk 256 bytes (not 1KB) seemed like a cool number to pick for the maximum amount in a "dan" packet.
				      // I think this is super low for today's standards but eh maybe it'll be cool having an old school networking library.

	// To organise m
	//char m_internalHeaderBytes[25];


	std::stringstream m_readBytes[maxPacketSize];

};

