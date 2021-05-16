#include "Client.h"

void Client::StartTimer()
{
	m_startTime = std::chrono::system_clock::now();
	m_isTimerStarted = true;
}

void Client::ResetTimer()
{
	m_isTimerStarted = false;
	m_elapsedTime = 0; // Clearing this back to 0 so it can be used again.
}

// Pass in the amount of time you want to trigger a true return.
bool Client::CheckTimer(double requiredMilliseconds)
{
	// Calculate the elapsed time so we can check it.
	m_endTime = std::chrono::system_clock::now();
	m_elapsedTime = (double)std::chrono::duration_cast<std::chrono::milliseconds>(m_endTime - m_startTime).count();

	if (m_elapsedTime >= requiredMilliseconds)
		return true;
	return false;
}
