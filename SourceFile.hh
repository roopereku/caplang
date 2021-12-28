#ifndef SOURCE_FILE_HEADER
#define SOURCE_FILE_HEADER

#include "TokenizedSource.hh"
#include "Scope.hh"

namespace Cap
{

class SourceFile
{
public:
	SourceFile(const std::string& path);
	bool validate();

	const std::vector <Token*> getImports() const;

private:
	std::vector <Token*> imports;

	TokenizedSource tokens;
	Scope root;
};

}

#endif
