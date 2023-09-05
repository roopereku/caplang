#ifndef CAP_SOURCE_FILE_HH
#define CAP_SOURCE_FILE_HH

#include <cap/Scope.hh>

#include <string_view>

namespace cap
{

class SourceFile
{
public:
	SourceFile(std::string_view path);

private:
	Scope global;
};

}

#endif
