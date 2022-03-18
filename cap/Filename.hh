#ifndef PATH_HEADER
#define PATH_HEADER

#include "Token.hh"

namespace Cap
{

struct Filename
{
	Filename(bool fromPath) :
		fromPath(fromPath)
	{
	}

	std::string name;
	bool fromPath;
};

}

#endif
