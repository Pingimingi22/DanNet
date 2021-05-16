#pragma once

#include <chrono>

/// ClientStruct is what server Peer's will have a container of. Each ClientStruct contains the id of the client aswell as the ip address (IPv4).

class Client 
{
public:
	Client() {};

	// Because Client used to be a struct, I'm manually filling these values in throughout the code base and to save myself time I'm going to keep it that way. So that's why these members are public.

	char m_ipAddress[25]; // hopefully 25 bytes is long enough.
	unsigned int m_clientID = 0;
	unsigned short m_port = 0;


	// --------------------- Client timeout stuff. --------------------- //
	bool m_isTimerStarted = false;
	double m_elapsedTime = 0; // Milliseconds.
	std::chrono::time_point<std::chrono::system_clock> m_startTime;
	std::chrono::time_point<std::chrono::system_clock> m_endTime;


	void StartTimer();
	void ResetTimer();
	bool CheckTimer(double requiredMilliseconds);
	// ----------------------------------------------------------------- //
};