#include "log.h"
#include <fstream>
#include <iostream>

Log::Log(std::string filename_, bool debug_) :
	buffer(""), filename(filename_), debug(debug_)
{ }

Log::~Log()
{
	std::ofstream file(filename);
	if (!file.is_open())
		return;

	file << buffer.str();
	file.close();

	if (debug)
	{
		std::cout << buffer.str();
		std::cin.get();
	}
}