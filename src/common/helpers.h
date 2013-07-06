#ifndef SLGL_COMMON_HELPERS_H
#define SLGL_COMMON_HELPERS_H
#define M_PI 3.14159265359f
#include <vector>
#include <string>
#include <sstream>

// Math
float degToRad(float degrees);
float radToDeg(float radians);

// Interpolators
float lerp(float a, float b, float t);
float bezier(float v0, float v1, float v2, float v3, float t);

// I/O
bool readFile(const char *filename, std::string &dest);

template <class T>
std::vector<T> split(const std::string &s, char delimiter)
{
	std::vector<T> ret;
	for(unsigned int i = 0; i < s.size(); ++i)
	{
		if(s[i] != delimiter)
		{
			unsigned int j = i;

			while(j < s.size() && s[j] != delimiter)
				++j;

			std::string sub = s.substr(i, j - i);

			std::stringstream ss(sub);
			T value;
			ss>>value;
			ret.push_back(value);

			i = j;
		}
	}

	return ret;
}

#endif