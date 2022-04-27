#include "SourceFile.hh"
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
		if(parseExpression(i, current) && !valid)
			return true;
	}

	return true;
}

bool Cap::SourceFile::validate()
{
	if(!valid)
		return false;

	ValidationResult result;

	if(!root.validate(result))
	{
		ERROR_LOG((*result.at->value), "%s\n", result.msg.c_str());
		return false;
	}

	DBG_LOG("Final result is %d", static_cast <int> (result.status));

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

bool Cap::SourceFile::showExpected(const std::string& msg, size_t& i)
{
	ERROR_LOG(tokens[i], "Expected %s\nInstead got ", msg.c_str());
	if(i < tokens.count())
	{
		printf("'%s' '%s'\n",
				tokens[i].getTypeString(),
				tokens[i].getString().c_str());
	}

	else printf("end of file\n");
	return true;
}
