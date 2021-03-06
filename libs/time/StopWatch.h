#pragma once

#include <chrono>

namespace util
{

/**
 * Helper class wrapping a std::chrono::steady_clock to measure
 * time intervals in seconds.
 * Use the getSecondsPassed() method to measure the time passed
 * since the last call to restart(). If no call to restart() was
 * made, it will return the time passed since construction.
 */
class StopWatch
{
private:
	std::chrono::steady_clock _clock;

	std::chrono::steady_clock::time_point _start;

public:
	StopWatch()
	{
		restart();
	}

	// Restarts the timer, resetting the time passed to 0
	void restart()
	{
		_start = _clock.now();
	}

	// Returns the seconds passed since the last call to restart()
	// If restart() has never been called, this is the duration since construction
	std::size_t getSecondsPassed() const
	{
		auto endTime = _clock.now();

		auto seconds = std::chrono::duration_cast<std::chrono::seconds>(endTime - _start).count();

		return static_cast<std::size_t>(seconds);
	}

	// Returns the milliseconds passed since the last call to restart()
	// If restart() has never been called, this is the duration since construction
	std::size_t getMilliSecondsPassed() const
	{
		auto endTime = _clock.now();

		auto msecs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - _start).count();

		return static_cast<std::size_t>(msecs);
	}
};

}
