#include "SourceFile.hh"
#include "Debug.hh"

Cap::SourceFile::SourceFile(const std::string& path)
	: tokens(path), root(nullptr, nullptr, ScopeContext::Block, 0, tokens.count())
{
	//	TODO exclude comments from tokens

	if(!tokens.matchBraces())
		return;

	for(size_t i = 0; i < root.end; i++)
	{
		if(parseExpression(i, root))
		{
			if(!valid)
				return;
		}
	}

	DBG_LOG("----------- LISTING NODES -------------%s", "");
	root.root.list();
}

bool Cap::SourceFile::validate()
{
	return true;
}

const std::vector <Cap::Filename> Cap::SourceFile::getImports() const
{
	return imports;
}

bool Cap::SourceFile::isToken(TokenType t, size_t& i)
{
	return i < tokens.count() && tokens[i].type == t;
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
