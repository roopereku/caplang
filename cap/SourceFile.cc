#include "SourceFile.hh"
#include "Logger.hh"
#include "Debug.hh"

Cap::SourceFile::SourceFile(const std::string& path)
	: tokens(path), root(nullptr, ScopeContext::Block, 0, tokens.count())
{
	//	TODO exclude comments from tokens

	if(!tokens.isValid() || !tokens.matchBraces())
		return;

	parseScope(root);
}

bool Cap::SourceFile::parseScope(Scope& current)
{
	for(size_t i = current.begin; i < current.end; i++)
	{
		if(parseLine(i, current) && !valid)
			return true;
	}

	return true;
}

bool Cap::SourceFile::validate()
{
	if(!valid)
		return false;

	if(!root.validate())
		return false;

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
	return	token.stringEquals("var") ||
			token.stringEquals("func") ||
			token.stringEquals("type");
}
