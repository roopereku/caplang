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

			//	TODO add the identifier
			DBG_LOG("Expression: Identifier '%s'", tokens[i].getString().c_str());
		}

		//	Check for numeric values
		else if(tokens[i].type != TokenType::Operator)
		{
			//	TODO add the value
			DBG_LOG("Expression: %s '%s'", tokens[i].getTypeString(), tokens[i].getString().c_str());
		}

		//	Check for operators
		else
		{
			DBG_LOG("Expression: Operator '%s'", tokens[i].getString().c_str());
			bool possiblyUnary = i == start || tokens[i - 1].type == TokenType::Operator;

			//	Is the next token the same operator as this one
			size_t next = i + 1;
			if(isToken(TokenType::Operator, next) && *tokens[next].begin == *tokens[i].begin)
			{
				//	FIXME When not in parenthesis, check if the line changes
				size_t gap = tokens.getIndex(next) - tokens.getIndex(i) - 1;
				DBG_LOG("Gap is %lu", gap);

				if(gap == 0)
				{
					DBG_LOG("Combine '%c'", *tokens[next].begin);

					switch(*tokens[next].begin)
					{
						//	Make sure that there's an identifiers on the appropriate side
						case '+': case '-':
						{
							break;
						}

						case '<': break;
						case '>': break;
						case '&': break;
						case '|': break;
						case '=': break;
						case '*': break;

						//	TODO make sure that there are identifiers on both sides
						case '.': break;

						default:
							ERROR_LOG(tokens[i], "Invalid operator '%c%c'\n", *tokens[i].begin, *tokens[i].begin);
							return true;
					}

					i = next;
					continue;
				}
			}

			else if(possiblyUnary)
			{
				DBG_LOG("Unary '%c'", *tokens[i].begin);
				continue;
			}

			DBG_LOG("Normal operator '%c'", *tokens[i].begin);
		}
	}

	inExpression = false;
	return true;
}

bool Cap::SourceFile::parseExpressionOrder(size_t begin, size_t end, Scope& current)
{
	return true;
}
