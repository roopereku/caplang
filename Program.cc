#include "Program.hh"
#include "Debug.hh"

Cap::Program::Program(const std::string& entryPath)
{
	DBG_LOG("Entry file is '%s'", entryPath.c_str());
	sources.emplace_back(entryPath);
}
