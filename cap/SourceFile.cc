#include "CodeGenerator.hh"
#include "SourceFile.hh"
#include "Logger.hh"
#include "Debug.hh"

Cap::SourceFile::SourceFile(const std::string& path)
	: tokens(path), root(nullptr, ScopeContext::Block)
{
	if(!tokens.isValid() || !tokens.matchBraces())
		return;

	size_t i = 0;
	parseScope(i, tokens.count(), root);
}

bool Cap::SourceFile::parseScope(size_t& i, size_t end, Scope& current)
{
	for(; i < end; i++)
	{
		std::string p = tokens[i].getString();
		//DBG_LOG("[%lu] parseScope '%s' '%s'", i, tokens[i].getTypeString(), p.c_str());

		if(!parseLine(i, current) && !valid)
			return false;
	}

	return true;
}

bool Cap::SourceFile::validate()
{
	if(!valid)
		return false;

	if(!root.validate())
		return false;

	CodeGenerator cg(root);
	return true;
}

const std::vector <Cap::Filename>& Cap::SourceFile::getImports() const
{
	return imports;
}

bool Cap::SourceFile::isToken(TokenType t, size_t& i)
{
	skipComments(i);
	return i < tokens.count() && tokens[i].type == t;
}

void Cap::SourceFile::skipComments(size_t& i)
{
	while(	tokens[i].type == TokenType::SingleLineComment ||
			tokens[i].type == TokenType::MultiLineComment)
	{
		i++;
	}
}

bool Cap::SourceFile::isKeyword(Token& token)
{
	return	token.stringEquals("if") ||
			token.stringEquals("for") ||
			token.stringEquals("var") ||
			token.stringEquals("while") ||
			token.stringEquals("func") ||
			token.stringEquals("type") ||
			token.stringEquals("import") ||
			Type::isPrimitiveName(&token);
}
