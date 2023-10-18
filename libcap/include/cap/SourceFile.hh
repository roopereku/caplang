#ifndef CAP_SOURCE_FILE_HH
#define CAP_SOURCE_FILE_HH

#include <cap/Scope.hh>
#include <cap/EventEmitter.hh>

#include <string_view>
#include <string>

namespace cap
{

class SourceFile
{
public:
	SourceFile(std::string_view path);

	const Scope& getGlobal()
	{
		return global;
	}

	bool parse(EventEmitter& events);
	bool validate(EventEmitter& events);

private:
	Scope global;

	std::string_view path;
	std::string source;
};

}

#endif
