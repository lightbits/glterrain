#include <common/timer.h>
#ifdef SLGL_WINDOWS
	#include <Windows.h>
#else
	#include <GL/glfw.h>
#endif

Timer::Timer() : startTime(0.0), prevTime(0.0), currTime(0.0), prevFpsUpdate(0.0), fps(0.0), frames(0), delta(0.0)
{

}

void Timer::start()
{
	startTime = getElapsedTime();
	prevTime = startTime;
	currTime = startTime;
	prevFpsUpdate = startTime;
}

void Timer::step()
{
	++frames;
	prevTime = currTime;
	currTime = getElapsedTime();

	delta = currTime - prevTime;

	double fpsTime = currTime - prevFpsUpdate;
	if(fpsTime > 1.0) // Update FPS every second
	{
		fps = (frames / fpsTime);
		prevFpsUpdate = currTime;
		frames = 0;
	}
}

double Timer::getElapsedTime() const
{
#ifdef SLGL_WINDOWS
	static __int64 freq = 0; // ticks per sec

	if(freq == 0)
	{
		LARGE_INTEGER freqTemp;
		QueryPerformanceFrequency(&freqTemp);
		freq = freqTemp.QuadPart;
	}

	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);

	// Conversion to double shouldn't be a problem unless you have an uptime of a couple
	// of decades
	return (double)(ticks.QuadPart / (double)freq);
#else
	return glfwGetTime();
#endif
}

double Timer::getDelta() const
{
	return currTime - prevTime;
}

double Timer::getFps() const
{
	return fps;
}