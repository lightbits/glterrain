#ifndef APP_LOG_H
#define APP_LOG_H
#include <sstream>
#include <string>

/*
A stream meant for logging errors, warnings and status messages.
It writes its contents to the filename given upon destruction,
and waits for input upon destruction if the 'debug' parameter is enabled.
It also prints to cout.
*/
class Log
{
public:
	Log(std::string filename, bool debug);
	~Log();

	template <typename T>
	Log &operator<<(T t)
	{
		buffer << t;
		return *this;
	}
private:
	bool debug;
	std::string filename;
	std::stringstream buffer;
};

#endif