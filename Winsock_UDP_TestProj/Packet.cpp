#include "Packet.h"

#include <sstream>
#include <assert.h>

#include <iostream>

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

void Packet::Read(int howManyBytes)
{
	m_readBytes->read(&m_allBytes[0], howManyBytes);
}

MessageIdentifier Packet::GetPacketIdentifier()
{
	int length = m_readBytes->str().length();
	assert(m_readBytes->str().length() > 0); // Making sure that we've atleast ready *something* in. This is really bad error checking but I guess it's better than nothing.
	char identifier;
	identifier = m_readBytes->str().at(1);

	int identifierNumeric = int(identifier);

	return (MessageIdentifier)identifierNumeric;

}
