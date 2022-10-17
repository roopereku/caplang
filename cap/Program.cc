#include "Program.hh"
#include "Debug.hh"

Cap::Program::Program(const std::string& entryPath)
{
	DBG_LOG("Entry file is '%s'", entryPath.c_str());
	parseSource(entryPath);
	sources.front().validate();
}

bool Cap::Program::parseSource(const std::string& path)
{
	sources.emplace_back(path);
	const std::vector <Filename>& imports = sources.back().getImports();

	for(auto& importPath : imports)
	{
		//	TODO handle the "from path" part
		if(!parseSource(importPath.name))
			return false;
	}

	return true;
}
