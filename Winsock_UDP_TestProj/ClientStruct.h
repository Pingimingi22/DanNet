#pragma once

/// ClientStruct is what server Peer's will have a container of. Each ClientStruct contains the id of the client aswell as the ip address (IPv4).

struct ClientStruct 
{
	char m_ipAddress[25]; // hopefully 25 bytes is long enough.
	unsigned int m_clientID = 0;
	unsigned short m_port = 0;
};