#ifndef SOURCE_FILE_HEADER
#define SOURCE_FILE_HEADER

#include "TokenizedSource.hh"
#include "Expression.hh"
#include "Filename.hh"
#include "Scope.hh"

namespace Cap
{

class SourceFile
{
public:
	SourceFile(const std::string& path);
	bool validate();

	const std::vector <Filename>& getImports() const;

private:
	bool parseImport(size_t& i, Scope& current);
	bool parseImportFilename(size_t& i);

	bool parseScope(Scope& current);
	bool parseLine(size_t& i, Scope& current, bool inBrackets = false);
	bool parseExpression(std::vector <ExpressionPart>& parts,
						 size_t offset, size_t end, size_t priority,
						 SyntaxTreeNode* node, Scope& current);

	bool parseLineInBracket(SyntaxTreeNode* node, Token* at, Scope& current);

	bool parseVariable(size_t& i, Scope& current);
	bool parseFunction(size_t& i, Scope& current);
	bool parseType(size_t& i, Scope& current);
	bool parseMisc(size_t& i, Scope& current);

	bool isToken(TokenType t, size_t& i);
	void skipComments(size_t& i);
	bool isKeyword(Token& token);
	bool errorOut();

	bool inExpression = false;
	bool valid = true;

	std::vector <Filename> imports;
	TokenizedSource tokens;
	Scope root;
};

}

#endif
