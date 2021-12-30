#include "SourceFile.hh"
#include "Debug.hh"

Cap::SourceFile::SourceFile(const std::string& path)
	: tokens(path), root(nullptr, nullptr, ScopeContext::Block)
{
	if(!tokens.matchBraces())
		return;

	for(size_t i = 0; i < tokens.count(); i++)
	{
		DBG_LOG("token %lu is '%s'", i, tokens[i].getString().c_str());

		if(parseImport(i, root))
		{
			if(!valid)
				return;
		}
	}
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

bool Cap::SourceFile::showExpected(const char* msg, size_t& i)
{
	printf("Error: Expected %s\nInstead got ", msg);
	if(i < tokens.count())
	{
		printf("'%s' '%s'\n",
				tokens[i].getTypeString(),
				tokens[i].getString().c_str());
	}

	else printf("end of file\n");

	valid = false;
	return true;
}
