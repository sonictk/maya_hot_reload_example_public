#ifndef TIMER_H
#define TIMER_H


#ifdef _WIN32
#include <Windows.h>

typedef LARGE_INTEGER HighResTimerVal;

#elif __linux__ || __APPLE__

typedef double HighResTimerVal;

#endif // Platform layer


/// Performance timer API
struct PerfTimer
{
	HighResTimerVal freq; // NOTE: (sonictk) On Linux/OSX, this is undefined
	HighResTimerVal start;
	HighResTimerVal end;
};


/// This acts as a global timer that can used for the host application.
globalVar PerfTimer kGlobalPerfTimer;

// TODO: (yliangsiew) Document forward declarations
int resetPerfTimerValue(PerfTimer &timer);


PerfTimer perfTimer();


int startPerfTimer(PerfTimer &timer);

int startPerfTimer();


int endPerfTimer(PerfTimer &timer);

int endPerfTimer();


double getPerfTimerValue(PerfTimer &timer);

double getPerfTimerValue();


#ifdef _WIN32

int resetPerfTimerValue(PerfTimer &timer)
{
	BOOL query = QueryPerformanceFrequency(&timer.freq);
	if (query == 0) {
		perror("Failed to initialize a timer!\n");
		return -1;
	}
	timer.start.QuadPart = 0;
	timer.end.QuadPart = 0;

	return 0;
}

PerfTimer perfTimer()
{
	PerfTimer result;
	resetPerfTimerValue(result);

	return result;
}

int startPerfTimer(PerfTimer &timer)
{
	BOOL result = QueryPerformanceCounter(&timer.start);
	if (result == 0) {
		perror("Starting the performance timer failed!\n");
		return -1;
	}

	return 0;
}

int startPerfTimer()
{
	kGlobalPerfTimer = perfTimer();
	return startPerfTimer(kGlobalPerfTimer);
}


int endPerfTimer(PerfTimer &timer)
{
	BOOL result = QueryPerformanceCounter(&timer.end);
	if (result == 0) {
		perror("Stopping the performance timer failed!\n");
		return -1;
	}

	return 0;
}

int endPerfTimer()
{
	return endPerfTimer(kGlobalPerfTimer);
}

double getPerfTimerValue(PerfTimer &timer)
{
	return (double)(timer.end.QuadPart - timer.start.QuadPart) * 1.0 / (double)timer.freq.QuadPart;
}

double getPerfTimerValue()
{
	return getPerfTimerValue(kGlobalPerfTimer);
}

#elif __linux__ || __APPLE__

// TODO: (sonictk) Eventually stop relying on stdlib
#include <chrono>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::time_point_cast;


// TODO: (sonictk) Test timer functions on linux
int resetPerfTimerValue(PerfTimer &timer)
{
	timer.freq = 0.0;
	timer.start = 0.0;
	timer.end = 0.0;

	return 0;
}

PerfTimer perfTimer()
{
	PerfTimer result;
	resetPerfTimerValue(result);

	return result;
}

int startPerfTimer(PerfTimer &timer)
{
	timer.start = time_point_cast<microseconds>(high_resolution_clock::now()).time_since_epoch().count();
	return 0;
}

int startPerfTimer()
{
	kGlobalPerfTimer = perfTimer();
	return startPerfTimer(kGlobalPerfTimer);
}

int endPerfTimer(PerfTimer &timer)
{
	timer.end = time_point_cast<microseconds>(high_resolution_clock::now()).time_since_epoch().count();
	return 0;
}

int endPerfTimer()
{
	return endPerfTimer(kGlobalPerfTimer);
}

double getPerfTimerValue(PerfTimer &timer)
{
	return timer.end - timer.start;
}

double getPerfTimerValue()
{
	return getPerfTimerValue(kGlobalPerfTimer);
}


#endif // Platform layer

#endif /* TIMER_H */
