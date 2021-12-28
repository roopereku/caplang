#ifndef PROGRAM_HEADER
#define PROGRAM_HEADER

#include "SourceFile.hh"

namespace Cap
{

class Program
{
public:
	Program(const std::string& entryPath);

private:
	std::vector <SourceFile> sources;
};

}

#endif
