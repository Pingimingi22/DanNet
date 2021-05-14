#pragma once

#include <sstream>

#include "MessageIdentifiers.h"

#include <iostream>

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"

#include "PacketPriorities.h"

//#include <stdint.h>

#include "combaseapi.h"

#include <chrono>

class Packet
{
	// Making peer a friend so that it can access SerializeInternal and DeserializeInternal.
	friend class Peer;
	friend class UDPListener;

	int32_t something;

public:
	Packet(int priority);

private:
	Packet() {} // We need a default constructor for Packet's because the UDPListener needs to be able to create a "generic" packet that it will fill in when it receives data.
	Packet(int priority, GUID guid); // Special constructor only to be used internally. NOT by the user. When the udp listener needs to send an ACK back, 
		                             //they will construct the packet with this constructor so they can hand pick the GUID.
public:
	Packet(const Packet& otherPacket)
	{
		this->m_priority = otherPacket.m_priority;
		for (int i = 0; i < 15; i++)
		{
			this->m_destinationIP[i] = otherPacket.m_destinationIP[i];
		}
		this->m_destinationPort = otherPacket.m_destinationPort;

		this->m_guid.Data1 = otherPacket.m_guid.Data1;
		this->m_guid.Data2 = otherPacket.m_guid.Data2;
		this->m_guid.Data3 = otherPacket.m_guid.Data3;

		for (int i = 0; i < 8; i++)
		{
			this->m_guid.Data4[i] = otherPacket.m_guid.Data4[i];
		}

		memcpy(&this->m_allBytes[0], &otherPacket.m_allBytes[0], 256);
		
	}

	Packet operator=(Packet otherPacket)
	{
		this->m_priority = otherPacket.m_priority;
		for (int i = 0; i < 15; i++)
		{
			this->m_destinationIP[i] = otherPacket.m_destinationIP[i];
		}
		this->m_destinationPort = otherPacket.m_destinationPort;

		this->m_guid.Data1 = otherPacket.m_guid.Data1;
		this->m_guid.Data2 = otherPacket.m_guid.Data2;
		this->m_guid.Data3 = otherPacket.m_guid.Data3;

		for (int i = 0; i < 8; i++)
		{
			this->m_guid.Data4[i] = otherPacket.m_guid.Data4[i];
		}

		memcpy(&this->m_allBytes[0], &otherPacket.m_allBytes[0], 256);

		return *this;
	}

public:
	void Create();
	void Send();
	//void SendReliable(); DEPRECATED

	// --------------------------------------------------- RELIABLE UDP TIMER STUFF --------------------------------------------------- //
	// ---------------------------------------------------      IMPORTANT NOTE      --------------------------------------------------- //
	//		I'll try use this timer stuff for lag send outs aswell. So these timers will be used for unreliable packets if the			//
	//		user wants to simulate lag.							
	//
	//		Because we are using the same variables for both sending out reliable and laggy packets, reliable packets CANNOT be
	//      in both the UpdateReliablePackets() function and UpdateLagSends() function since the variables will write over each other.
	//	    Right now the idea is that reliable packets will NOT be placed in the m_lagPacketQueue so there shouldn't be any issues.
	// -------------------------------------------------------------------------------------------------------------------------------- //

	// - Every packet will have the ability to set and use these timers but I will only use them for the packets in the reliable packet queue (not anymore, also laggy packets will use it).
	// - The reason I'm using chrono steady_clock instead of system_clock is because system_clock can be changed by the user at any time. (using system_clock for now, have to run some tests on steady_clock.)

	// To be used when sending reliable udp packets. Starting a packet's timer will set it's start time. We can then choose to send out packets which have counted "x" amount of seconds.
	void StartPacketTimer();
	// Sets the m_endTime of the timer so we can get the different between end and start to calculate how long it's been.
	void CheckPacketTimer();
	// Gets the difference between when we started the timer and when we ended it. Set's the value of m_elapsedMilliseconds.
	void GetTimeDuration();

	void StopPacketTimer();

	std::chrono::time_point<std::chrono::system_clock> m_startTime;
	std::chrono::time_point<std::chrono::system_clock> m_endTime; // Not necessarily always the true end point. It's mostly going to be used to test if the current time has elapsed a certain amount of seconds.

	bool m_isTimerStarted = false;

	double m_elapsedMilliseconds = 0;
	// -------------------------------------------------------------------------------------------------------------------------------- //

	// ----------------- STUFF FOR RELIABLE UDP WITH SERVERS ----------------- //
	// Because server's don't "connect" they can't use the regular Send() function but right now reliable udp packets are continusously sent with Send(). I need a way to cache the ip address
	// so I can use SendTo() instead.
	char m_destinationIP[15];
	unsigned short m_destinationPort;

	
	bool m_hasSpecifiedDestination;

	// Set's the m_destinationIP and m_destinationPort so that I don't have to do it manually. This function will also set m_hasSpecifiedDestination to true which will mean DanNet will send with SendTo() rather than Send() in some places.
	void SetDestination(const char* ipAddress, unsigned short portNumber);
	// ----------------------------------------------------------------------- //



	void Clear(); // I don't know why I made this function, it was supposed to be called to delete packet's but I feel like calling delete explicitily is easier for others to read.
	

	// GetPacketPriority() gets the first four bytes (the true first four bytes, this should only ever be used internally and never by the user.)
	PacketPriority GetPacketPriority();

	// GetPacketIndentifier() gets the first four bytes for the user. It will return the type of packet in enum form.
	MessageIdentifier GetPacketIdentifier(); // Only to be used after one byte has been read from the packet.

	// To help with things, packets cache their priority and guid to make it easier for other functions to utilise without having to deserialise m_bytes.
	PacketPriority m_priority;
	GUID m_guid;


	// ----------------------------- Derserialize verdaic function unpacking technique in progress here! ----------------------------- //
	// Unfortunately function parameter pack's cannot be iterated through and we have to do a recursive technique that involves having a base case (the first Deserialize() function and a recursive
	// case the second Deserialize() function).

	// --------> Helpful resource to understand this: https://kevinushey.github.io/blog/2016/01/27/introduction-to-c++-variadic-templates/

	std::stringstream m_recursiveStream; // make sure to clear this after use.
	//cereal::BinaryOutputArchive* testOutput = nullptr;
	//cereal::BinaryInputArchive* testInput = nullptr;

	template<typename T>
	void Deserialize(T& t)
	{
		//std::stringstream ss; // ------------> Unfortunately we can't create streams inside these functions because the base case and the resursive case functions need to use the same stream. :(
		//m_recursiveStream.write(&m_allBytes[0], 1024);

		cereal::BinaryInputArchive iarchive(m_recursiveStream);
		
		iarchive(t);
		
		//testInput->operator()(t);
		
		//std::cout << 1 << std::endl;
		m_recursiveStream.clear();	

		//delete testInput;
		//testInput = nullptr;
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



		//testInput->operator()(first);
		

		Deserialize(args...);
	}
	// ------------------------------------------------------------------------------------------------------------------------------- //

	// --------------------------------------- More veriadic functions here. This time it's serializing. --------------------------------------- //
 	
	template<typename T>
	void Serialize(T& t)
	{
		cereal::BinaryOutputArchive outputArchive(m_recursiveStream);
		outputArchive(t);

		//testOutput->operator()(t);


		m_recursiveStream.read(&m_allBytes[0], 256);
		m_recursiveStream.clear();

		//delete testOutput;
		//testOutput = nullptr;
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

		//testOutput = new cereal::BinaryOutputArchive(m_recursiveStream);
		
		//testOutput->operator()(first);
		
		

		Serialize(args...);
	}

	// ----------------------------------------------------------------------------------------------------------------------------------------- //
private:

	void InternalHeaderSerialize(int& Priority, GUID* guid = nullptr)
	{


		// I need to quickly serialize the header data before serializing the user's payload data.
		// But I only know what to put in the header data after the user has already serialized their stuff.

		cereal::BinaryOutputArchive outputArchive(m_recursiveStream);

		//testOutput = new cereal::BinaryOutputArchive(m_recursiveStream);

		GUID testGuid;
		memset(&testGuid, 0, sizeof(GUID));


	
		// Serializing the packet priority.
		outputArchive(Priority);
		//testOutput->operator()(Priority);

		// Serializing the unique GUID. We only need to generate a GUID if the parsed in priority is reliable udp.
		if (Priority == (int)PacketPriority::RELIABLE_UDP && guid == nullptr) // This means they don't want a specific GUID, so we'll create a new one for them.
		{
			CoCreateGuid(&testGuid);
			m_guid.Data1 = testGuid.Data1;
			m_guid.Data2 = testGuid.Data2;
			m_guid.Data3 = testGuid.Data3;
			for (int i = 0; i < 8; i++)
			{
				m_guid.Data4[i] = testGuid.Data4[i];
			}
			//memcpy(&m_guid, &testGuid, sizeof(testGuid)); // giving the packet a cache of the guid so we can use it for easy comparisons in the UDPListener.
			// Not sure if memcpy is safe but whatev lol
		}
		else if (guid != nullptr) // This means the parsed in a GUID so we'll set this GUID to theirs. It's probably a server sending an ACK.
		{
			//memcpy(&testGuid, &guid, sizeof(GUID)); // putting the stuff into our cache of testGuid.
			testGuid.Data1 = guid->Data1;																			// ==================== NOTE ==================== //
			testGuid.Data2 = guid->Data2;																			// Unfortunately memcpy doesn't work for copying  //
			testGuid.Data3 = guid->Data3;																			// parts of one GUID struct to another so I'm     //
			for (int i = 0; i < 8; i++)																				// doing it manually.                             //      
			{																										// ============================================== //
				testGuid.Data4[i] = guid->Data4[i];
			}
			//memcpy(&m_guid, &testGuid, sizeof(testGuid)); // now putting it into the packet's cache.
			// 
			// giving the packet of a copy of the guid in type form. So this is in addition to the binary that will get sent across.
			// The packet's copy of the guid in type form is just used for easy comparisons elsewhere in the code base like in UDPListener.cpp.
			m_guid.Data1 = guid->Data1;
			m_guid.Data2 = guid->Data2;
			m_guid.Data3 = guid->Data3;
			for (int i = 0; i < 8; i++)
			{
				m_guid.Data4[i] = guid->Data4[i];
			}
		}

		outputArchive(testGuid.Data1);
		outputArchive(testGuid.Data2);
		outputArchive(testGuid.Data3);

		//testOutput->operator()(testGuid.Data1);
		//testOutput->operator()(testGuid.Data2);
		//testOutput->operator()(testGuid.Data3);
		

		for (int i = 0; i < 8; i++)
		{
			outputArchive(testGuid.Data4[i]);
			//testOutput->operator()(testGuid.Data4[i]);
		}

		//m_recursiveStream.read(&m_allBytes[0], 256);
		//m_recursiveStream.clear(); // important. the deserialization step can't happen unless we free up our recursiveStream.

	}

	public:
	void InternalHeaderDeserialize(int& priority, GUID& guid)
	{


		m_recursiveStream.write(&m_allBytes[0], 256);

		cereal::BinaryInputArchive inputArchive(m_recursiveStream);
		
		inputArchive(priority);

		//testInput = new cereal::BinaryInputArchive(m_recursiveStream);
		//testInput->operator()(priority);

		inputArchive(guid.Data1);
		inputArchive(guid.Data2);
		inputArchive(guid.Data3);
		//testInput->operator()(guid.Data1);
		//testInput->operator()(guid.Data2);
		//testInput->operator()(guid.Data3);


		for (int i = 0; i < 8; i++)
		{
			inputArchive(guid.Data4[i]);
			//testInput->operator()(guid.Data4[i]);
		}

	}

	private:
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

