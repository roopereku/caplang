#include "SourceFile.hh"
#include "Debug.hh"

bool Cap::SourceFile::parseExpression(size_t& i, Scope& current)
{
	/*	If the first token is an identifier, we could have a
	 *	declaration or an import. In that case we might
	 *	not yet be inside an expression */
	inExpression = !isToken(TokenType::Identifier, i);
	size_t start = i;

	for(; i < current.end; i++)
	{
		DBG_LOG("expr %s '%s'", tokens[i].getTypeString(), tokens[i].getString().c_str());
		size_t currentLine = tokens[i].line;

		if(tokens[i].type == TokenType::Break)
			break;

		else if(tokens[i].type == TokenType::Identifier)
		{
			size_t old = i;

			//	Are there any declarations or imports
			if(parseImport(i, current) || parseType(i, current) ||
				parseVariable(i, current) || parseFunction(i, current))
			{
				DBG_LOG("old is %lu  start is %lu", old, start);

				//	If something interrupts an expression, error out
				if(old > start)
					return showExpected("';' or a new line", old);

				/*	There was a valid declaration or import.
				 *	Let's exit the function to get a clean state */
				else return true;
			}

			//	At this point we're definitely inside an expression
			inExpression = true;
		}
	}

	inExpression = false;
	return true;
}

bool Cap::SourceFile::parseExpressionOrder(size_t begin, size_t end, Scope& current)
{
	return true;
}
