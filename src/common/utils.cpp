#include <common/utils.h>
#include <common/typedefs.h>
#include <ctype.h>
#include <sstream>
#include <fstream>

float degToRad(float degrees) 
{
	return degrees * PI / 180.0f; 
}

float radToDeg(float radians) 
{
	return radians * 180.0f / PI; 
}

float lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

float bezier(float v0, float v1, float v2, float v3, float t)
{
	float w0 = lerp(v0, v1, t);
	float w1 = lerp(v1, v2, t);
	float w2 = lerp(v2, v3, t);

	float q0 = lerp(w0, w1, t);
	float q1 = lerp(w1, w2, t);

	return lerp(q0, q1, t);
}

bool readFile(const std::string &filename, std::string &dest)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if(!in.is_open())
		return false;

	if(in.good())
	{
		in.seekg(0, std::ios::end);			// Set get position to end
		dest.resize(in.tellg());			// Resize string to support enough bytes
		in.seekg(0, std::ios::beg);			// Set get position to beginning
		in.read(&dest[0], dest.size());		// Read file to string
		in.close();
	}

	return true;
}

bool readFile(const std::string &filename, std::vector<std::string> &dest)
{
	std::ifstream in(filename, std::ios::in);
	dest.clear();
	if(!in.is_open())
		return false;

	dest.clear();
	while(in.good())
	{
		std::string line;
		std::getline(in, line);
		dest.push_back(line);
	}
	in.close();

	return true;
}

bool fileExists(const std::string &filename)
{
	std::ifstream in(filename, std::ios::in);
	bool ok = in.good();
	in.close();
	return ok;
}

bool endsWith(const std::string &s, const std::string &end)
{
	std::string::size_type e = end.size() - 1;
	std::string::size_type t = s.size() - 1;
	while(e >= 0 && t >= 0)
	{
		if(s[t] != end[e])
			return false;
		e--;
		t--;
	}
	return true;
}