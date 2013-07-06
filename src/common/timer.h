#ifndef SLGL_COMMON_TIMER_H
#define SLGL_COMMON_TIMER_H

/*
See http://msdn.microsoft.com/en-us/library/windows/desktop/ms644900(v=vs.85).aspx
for high-res timers on the windows platform
*/

class Timer
{
public:
	Timer();
	void start();
	void step();

	// Returns the time in seconds passed since last call to step
	double getDelta() const;

	// Returns elapsed time (in seconds) since last call to start
	double getElapsedTime() const;

	double getFps() const;
private:
	double startTime;
	double prevTime;
	double currTime;
	double prevFpsUpdate;
	double fps;
	double delta;
	int frames;
};

#endif