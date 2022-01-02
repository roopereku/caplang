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
	bool parseImport(size_t& i, Scope& current);
	bool parseImportFilename(size_t& i);

	bool parseExpression(size_t& i, Scope& current);
	bool parseExpressionOrder(size_t begin, size_t end, Scope& current);

	bool parseVariable(size_t& i, Scope& current);
	bool parseFunction(size_t& i, Scope& current);
	bool parseType(size_t& i, Scope& current);

	bool showExpected(const std::string& msg, size_t& i);
	bool isToken(TokenType t, size_t& i);

	bool inExpression = false;
	bool valid = true;

	std::vector <Filename> imports;
	TokenizedSource tokens;
	Scope root;
};

}

#endif
