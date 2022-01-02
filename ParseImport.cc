#include "SourceFile.hh"
#include "Debug.hh"

bool Cap::SourceFile::parseImport(size_t& i, Scope& current)
{
	if(!tokens[i].stringEquals("import"))
		return false;

	else if(inExpression)
		return true;

	/*	Though having an import not in the global scope is totally
	 *	functional, it looks bad and doesn't make much sense */
	if(current.parent)
	{
		printf("Error: Imports should only appear in the global scope\n");
		valid = false;
		return true;
	}

	i++;

	//	When there's a curly brace, there could be multiple filenames
	if(isToken(TokenType::CurlyBrace, i))
	{
		//	Loop until the matching curly brace is found
		for(i++; !isToken(TokenType::CurlyBrace, i); i++)
		{
			//	The return value is validity so stop if there were errors
			if(!parseImportFilename(i))
				return true;

			//	Is the next token the matching brace
			if(tokens[i + 1].length == 0)
				continue;

			//	Is the next token a comma
			else if(tokens[i + 1].type == TokenType::Break)
			{
				i++;
				if(*tokens[i].begin == ',')
					continue;
			}

			//	The next token is something unexpected
			i++;
			return showExpected("',' or '}' after filename", i);
		}
	}

	//	When there's no curly brace, there should be one filename
	else parseImportFilename(i);

	return true;
}

bool Cap::SourceFile::parseImportFilename(size_t& i)
{
	if(isToken(TokenType::String, i))
	{
		//	Add a filename that is not in the path
		imports.emplace_back(false);
		imports.back().name += tokens[i].getString();
	}

	else if(isToken(TokenType::Identifier, i))
	{
		//	Add a filename that is in the path
		imports.emplace_back(true);

		for(; i < tokens.count(); i++)
		{
			if(isToken(TokenType::Operator, i))
			{
				//	Dots are the only operators allowed in filenames found in the path
				if(*tokens[i].begin != '.')
					return showExpected("'.' to separate filename and directory", i);

				imports.back().name += '/';
				size_t next = i + 1;

				//	Make sure that the dot isn't the end of the filename
				if(!isToken(TokenType::Identifier, next))
					return showExpected("a filename or directory after '.'", next);
			}

			else if(isToken(TokenType::Identifier, i))
			{
				imports.back().name += tokens[i].getString();
				size_t next = i + 1;

				//	If there isn't an operator after a filename/directory, stop parsing
				if(!isToken(TokenType::Operator, next))
					break;
			}

			//	Only dots and identifiers are allowed
			else return showExpected("a filename/directory or '.'", i);
		}
	}

	//	Only identifiers and string are allowed as the first token in the filename
	else return showExpected("Expected a filename for import", i);

	imports.back().name += ".cap";
	DBG_LOG("filename '%s'", imports.back().name.c_str());
	return valid;
}
