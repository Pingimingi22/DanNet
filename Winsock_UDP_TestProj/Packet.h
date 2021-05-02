#pragma once

#include <sstream>

#include "MessageIdentifiers.h"

#include <iostream>

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"



class Packet
{
public:
	Packet();
	void Create();
	void Send();
	void SendReliable();

	

	MessageIdentifier GetPacketIdentifier(); // Only to be used after one byte has been read from the packet.


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
		m_recursiveStream.write(&m_allBytes[0], 1024);				// kind of confused why this is even working. Realistically, I shouldn't be rewriting everything to the string stream each type this    //
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

		m_recursiveStream.read(&m_allBytes[0], 1024);
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
	static constexpr int maxPacketSize = 1024;

	void Write(int howManyBytes);

public:
	// These probably shouldn't be public but it's easier this way to read things into these byte arrays all the way in the UDPListener.

	char m_allBytes[maxPacketSize]; // idk 1024 bytes (1KB) seemed like a cool number to pick for the maximum amount in a "dan" packet.
				      // I think this is super low for today's standards but eh maybe it'll be cool having an old school networking library.
	std::stringstream m_readBytes[maxPacketSize];

};

