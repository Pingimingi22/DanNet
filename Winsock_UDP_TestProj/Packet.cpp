#include "Packet.h"

#include <sstream>
#include <assert.h>

#include <iostream>

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"



Packet::Packet(int priority)
{

	m_priority = (PacketPriority)priority;
	InternalHeaderSerialize(priority); // applying internal headers to the packet.
	
}

Packet::Packet(int priority, GUID guid)
{
	m_priority = (PacketPriority)priority;
	InternalHeaderSerialize(priority, &guid); // applying internal headers but with a specific GUID. used for sending ACK's.
}

void Packet::Create()
{
}

void Packet::Send()
{
}

void Packet::SendReliable()
{
}

void Packet::StartPacketTimer()
{
	m_startTime = std::chrono::system_clock::now();
	m_isTimerStarted = true;
}

void Packet::CheckPacketTimer()
{
	m_endTime = std::chrono::system_clock::now();
}

void Packet::GetTimeDuration()
{
	// We better hope we've set the start and end times before we call this function.
	m_elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(m_endTime - m_startTime).count();
}

void Packet::StopPacketTimer()
{
	m_isTimerStarted = false; // resetting the boolean so we can start the timer again in the update function.
	m_elapsedMilliseconds = 0; // resetting elapsed milliseconds to avoid errors.
}

void Packet::Clear()
{
	delete this;
}

void Packet::Write(int howManyBytes)
{
	m_readBytes->write(m_allBytes, howManyBytes);
}

PacketPriority Packet::GetPacketPriority()
{

	// the first byte will be the packet priority type. the next two bytes will be random numbers specific to the packet so the receiver can confirm whether it arrived or not.
	int packetPriority = int(m_allBytes[0]);
	return (PacketPriority)packetPriority;
}

MessageIdentifier Packet::GetPacketIdentifier()
{
	//int length = m_readBytes->str().length();
	//assert(m_readBytes->str().length() > 0); // Making sure that we've atleast ready *something* in. This is really bad error checking but I guess it's better than nothing.
	//unsigned int identifier = 0;
	//m_readBytes->read((char*)&identifier, sizeof(char)); // sizeof char because char's are 1 byte.
	//
	//int identifierNumeric = int(identifier);
	//return (MessageIdentifier)identifierNumeric;

	// Testing deserializing to get the packet identifier.
	std::stringstream testStream;
	testStream.write(&m_allBytes[0], 256);

	cereal::BinaryInputArchive testInputArchive(testStream);

	int thingWeDontCareAbout1;
	GUID thingWeDontCareAbout2;
	memset(&thingWeDontCareAbout2, 0, sizeof(GUID));
	int packetIdentifier;
	testInputArchive(thingWeDontCareAbout1, thingWeDontCareAbout2.Data1, thingWeDontCareAbout2.Data2, thingWeDontCareAbout2.Data3);
	for (int i = 0; i < 8; i++)
	{
		testInputArchive(thingWeDontCareAbout2.Data4[i]);
	}

	testInputArchive(packetIdentifier);
	// The idea behind adding the size of an int and the size of a GUID is so that we go passed all my internal header data stuff and get to the packet type bytes.
	int testIdentifier = (int)m_allBytes[sizeof(int) + 16 ];
	//return (MessageIdentifier)testIdentifier;

	// new packet identifier that probably wont work!
	return (MessageIdentifier)packetIdentifier;

}