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

void Packet::SetDestination(const char* ipAddress, unsigned short portNumber)
{
	strcpy_s(m_destinationIP, ipAddress);
	m_destinationPort = portNumber;
	m_hasSpecifiedDestination = true;
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
	std::stringstream testStream;
	testStream.write(&m_allBytes[0], 256);

	cereal::BinaryInputArchive testInputArchive(testStream);

	int thingWeDontCareAbout1; // This would be the packet priority.
	GUID thingWeDontCareAbout2; // This would be the GUID.
	memset(&thingWeDontCareAbout2, 0, sizeof(GUID));
	int packetIdentifier;
	testInputArchive(thingWeDontCareAbout1, thingWeDontCareAbout2.Data1, thingWeDontCareAbout2.Data2, thingWeDontCareAbout2.Data3);
	for (int i = 0; i < 8; i++)
	{
		testInputArchive(thingWeDontCareAbout2.Data4[i]);
	}

	testInputArchive(packetIdentifier);

	
	return (MessageIdentifier)packetIdentifier;

}