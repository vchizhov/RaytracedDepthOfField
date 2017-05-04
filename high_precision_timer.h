#pragma once
#include <Windows.h>

class HighPrecisionTimer
{
private:
	float invFrequency;        // seconds per tick
	LARGE_INTEGER t1;         // ticks

public:
	HighPrecisionTimer()
	{
		// get ticks per second
		QueryPerformanceFrequency(&t1);

		invFrequency = 1.0f/t1.QuadPart;
	}

	float getTime()
	{
		QueryPerformanceCounter(&t1);
		return t1.QuadPart*invFrequency;
	}
		
	
};