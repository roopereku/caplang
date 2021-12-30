#ifndef SOURCE_FILE_HEADER
#define SOURCE_FILE_HEADER

#include "TokenizedSource.hh"
#include "Filename.hh"
#include "Scope.hh"

namespace Cap
{

class SourceFile
{
public:
	SourceFile(const std::string& path);
	bool validate();

	const std::vector <Filename> getImports() const;

private:
	bool parseImport(size_t& i);
	bool parseImportFilename(size_t& i);

	bool isType(TokenType t, size_t& i);
	bool showExpected(const char* msg, size_t& i);

	bool valid = true;
	std::vector <Filename> imports;
	TokenizedSource tokens;
	Scope root;
};

}

#endif
