#include "Packet.h"

#include <sstream>
#include <assert.h>

#include <iostream>

#include "cereal/cereal.hpp"
#include "cereal/archives/binary.hpp"

Packet::Packet()
{
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

void Packet::Write(int howManyBytes)
{
	m_readBytes->write(m_allBytes, howManyBytes);
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


	int testIdentifier = (int)m_allBytes[0];
	return (MessageIdentifier)testIdentifier;

}