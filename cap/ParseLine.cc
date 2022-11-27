#include "SourceFile.hh"
#include "Logger.hh"
#include "Debug.hh"

bool Cap::SourceFile::errorOut()
{
	valid = false;
	return valid;
}

bool Cap::SourceFile::parseLine(size_t& i, Scope& current, bool inBrackets)
{
	SyntaxTreeNode* lastNode = current.node;

	/*	If the first token is an identifier, we could have a
	 *	declaration or an import. In that case we might
	 *	not yet be inside an expression */
	inExpression = !isToken(TokenType::Identifier, i);

	size_t start = i;
	size_t startLine = tokens[i].line;

	bool expectVariableName = false;
	bool lastWasOperator = false;

	std::vector <ExpressionPart> parts;

	//	If the current node is "Variable", declare variables implicitly
	if(current.node->type == SyntaxTreeNode::Type::Variable)
	{
		DBG_LOG("IMPLICIT DECLARATIONS");
		expectVariableName = true;
		inExpression = true;
	}

	for(; i < tokens.count(); i++)
	{
		skipComments(i);

		std::string p = tokens[i].getString();
		DBG_LOG("[%lu] line '%s' '%s'", i, tokens[i].getTypeString(), p.c_str());

		//	Is the token a closing bracket or a semicolon?
		if(tokens[i].length == 0 || tokens[i].type == TokenType::Break)
		{
			if(inBrackets && tokens[i].type == TokenType::Break)
			{
				Logger::error(tokens[i], "Can't use ';' inside brackets");
				return errorOut();
			}

			break;
		}

		//	Is a variable name expected
		if(expectVariableName)
		{
			if(!isToken(TokenType::Identifier, i))
			{
				Logger::error(tokens[i], "Expected a name for a variable");
				return errorOut();
			}

			expectVariableName = false;
			size_t next = i + 1;

			//	If the next token isn't '=', report an error
			if(!isToken(TokenType::Operator, next) || *tokens[next].begin != '=')
			{
				Logger::error(tokens[i], "Expected an initial value or type for variable '%s'", tokens[i].getString().c_str());
				return errorOut();
			}

			startLine = tokens[next].line;
			current.addVariable(&tokens[i]);
			parts.push_back({ SyntaxTreeNode::Type::Value, &tokens[i] });

			continue;
		}

		//	Has the line changed? (Not applicable if inside brackets)
		if(!inBrackets && inExpression && tokens[i].line > startLine)
		{
			//	Expressions can be multiline if a line break happens after an operator
			if(!lastWasOperator)
			{
				DBG_LOG("LINE BREAK");
				i--;
				break;
			}

			else startLine = tokens[i].line;
		}

		if(tokens[i].type == TokenType::Identifier)
		{
			bool oldInExpression = inExpression;
			size_t old = i;

			/*	Though this is a little cryptic, store the function index to
			 *	the token's length. This terminates the need for a non-generic
			 *	number in SyntaxTreeNode */
			if(parseFunction(i, current))
			{
				Function* f = current.findFunction(current.getFunctionCount() - 1);

				inExpression = oldInExpression;
				bool anonymous = f->name ? false : true;

				//	Don't continue with an expression if the function is anonymous
				if(!anonymous) return true;

				//	If parseFunction() failed, stop here
				if(!valid) return errorOut();

				tokens[old].length = current.getFunctionCount() - 1;
				parts.push_back({ SyntaxTreeNode::Type::AnonFunction, &tokens[old] });

				lastWasOperator = false;
				continue;
			}

			//	Is there a variable declaration?
			else if(parseVariable(i, current))
			{
				//	Nothing happened because we're inside an expression
				if(i == old)
				{
					Logger::error(tokens[i], "Can't declare a variable inside an expression");
					return errorOut();
				}

				DBG_LOG("VARIABLE MODE");

				inExpression = true;
				expectVariableName = true;
				i--;

				continue;
			}

			if( parseImport(i, current) ||
				parseType(i, current))
			{
				//if(i == old)
				break;
			}

			if(parseMisc(i, current))
			{
				if(!valid) return errorOut();
				break;
			}

			//	At this point the identifier is just a value
			parts.push_back({ SyntaxTreeNode::Type::Value, &tokens[i] });

			lastWasOperator = false;
			inExpression = true;
		}

		else if(tokens[i].type != TokenType::Operator)
		{
			SyntaxTreeNode::Type which = SyntaxTreeNode::Type::None;

			bool skip = true;
			bool lastIdentifier = !parts.empty() && parts.back().value->type == TokenType::Identifier;

			DBG_LOG("Last identifier %d", lastIdentifier);

			if(lastIdentifier)
			{
				switch(tokens[i].type)
				{
					case TokenType::Parenthesis: which = SyntaxTreeNode::Type::Call; break;
					case TokenType::SquareBracket: which = SyntaxTreeNode::Type::Subscript; break;

					case TokenType::CurlyBrace:
						Logger::error(tokens[i], "TODO: Implemet curly braces after identifiers");
						return errorOut();

					default: break;
				}

				if(which != SyntaxTreeNode::Type::None)
				{
					DBG_LOG("OVERRIDE '%s' to '%s'", parts.back().value->getString().c_str(), SyntaxTreeNode::getTypeString(which));
					parts.back().type = which;
					i = i + tokens[i].length;
					continue;
				}
			}

			//	If the last part wasn't an operator, there can't be a value here
			if(i > start && !lastWasOperator)
			{
				Logger::error(tokens[i], "Expected an operator before value '%s'", tokens[i].getString().c_str());
				return errorOut();
			}

			switch(tokens[i].type)
			{
				//	Brackets need special attention if they are values
				case TokenType::Parenthesis: which = SyntaxTreeNode::Type::Parentheses; break;
				case TokenType::SquareBracket: which = SyntaxTreeNode::Type::Array; break;

				case TokenType::CurlyBrace:
					Logger::error(tokens[i], "TODO: Implement curly braces as a value");
					return errorOut();
				break;

				//	The value is anything else but brackets
				default:
					which = SyntaxTreeNode::Type::Value;
					skip = false;
				break;
			}

			parts.push_back({ which, &tokens[i] });
			lastWasOperator = false;

			//	If the value was defined inside brackets, skip the brackets for now
			if(skip) i = i + tokens[i].length;
		}

		else
		{
			SyntaxTreeNode::Type which = SyntaxTreeNode::Type::None;

			size_t next = i + 1;
			bool nextIsOperator = isToken(TokenType::Operator, next);

			//	There can only be a double operator when the operators don't have anything in between
			if(nextIsOperator && tokens.getBeginIndex(next) - tokens.getBeginIndex(i) > 1)
				nextIsOperator = false;

			//	An operator is unary if the last token is an operator or if this is the first token
			if(!nextIsOperator && (lastWasOperator || parts.empty()))
			{
				switch(*tokens[i].begin)
				{
					case '+': which = SyntaxTreeNode::Type::UnaryPositive; break;
					case '-': which = SyntaxTreeNode::Type::UnaryNegative; break;

					case '!': which = SyntaxTreeNode::Type::Not; break;
					case '@': which = SyntaxTreeNode::Type::Reference; break;
					case '~': which = SyntaxTreeNode::Type::BitwiseNOT; break;

					default:
						Logger::error(tokens[i], "Invalid unary operator '%c'", *tokens[i].begin);
						return errorOut();
				}

				parts.push_back({ which, &tokens[i] });
				lastWasOperator = true;

				continue;
			}

			//	FIXME somehow permit ++ and --
			//	Operators can't come before values unless they're unary
			else if(parts.empty())
			{
				Logger::error(tokens[i], "Expected a value before an operator");
				return errorOut();
			}

			//	Each type before Not is an operator that requires a value after it
			if(lastWasOperator && parts.back().type < SyntaxTreeNode::Type::Not)
			{
				Logger::error(tokens[i], "Expected a value");
				return errorOut();
			}

			bool hasExtension = false;
			lastWasOperator = true;

			//	Is the next token an operator?
			if(nextIsOperator)
			{
				//	Are this and the next token identical?
				if(*tokens[i].begin == *tokens[next].begin)
				{
					next++;

					//	'=' extension are allowed if the next token is '='
					bool allowExtension = isToken(TokenType::Operator, next) && *tokens[next].begin == '=';
					bool canExtend = false;

					switch(*tokens[i].begin)
					{
						case '<': which = SyntaxTreeNode::Type::BitwiseShiftLeft; canExtend = true; break;
						case '>': which = SyntaxTreeNode::Type::BitwiseShiftRight; canExtend = true; break;
						case '*': which = SyntaxTreeNode::Type::Power; canExtend = true; break;

						case '=': which = SyntaxTreeNode::Type::Equal; break;
						case '&': which = SyntaxTreeNode::Type::And; break;
						case '|': which = SyntaxTreeNode::Type::Or; break;

						case '.': which = SyntaxTreeNode::Type::Range; break;

						default:
							Logger::error(tokens[i], "Invalid operator '%c%c'", *tokens[i].begin, *tokens[i].begin);
							return errorOut();
					}

					//	If '=' extension is allowed, don't jump to the next iteration
					if(!allowExtension || !canExtend)
					{
						//	The operator is 2 characters long
						tokens[i].length = 2;

						parts.push_back({ which, &tokens[i] });
						i = next - 1;
						continue;
					}

					//	The operator is 3 characters long
					tokens[i].length = 3;
				}

				//	There are no more >1 character operators left that don't end with '='
				if(*tokens[next].begin == '=')
				{
					//	If operator type hasn't been set yet, the operator is 2 characters long
					if(which == SyntaxTreeNode::Type:: None) tokens[i].length = 2;
					hasExtension = true;
				}
			}

			//	The type of the operator might have been set earlier
			if(which == SyntaxTreeNode::Type::None)
			{
				switch(*tokens[i].begin)
				{
					case '+': which = SyntaxTreeNode::Type::Addition; break;
					case '-': which = SyntaxTreeNode::Type::Subtraction; break;
					case '*': which = SyntaxTreeNode::Type::Multiplication; break;
					case '/': which = SyntaxTreeNode::Type::Division; break;
					case '%': which = SyntaxTreeNode::Type::Modulus; break;

					case '&': which = SyntaxTreeNode::Type::BitwiseAND; break;
					case '^': which = SyntaxTreeNode::Type::BitwiseXOR; break;
					case '|': which = SyntaxTreeNode::Type::BitwiseOR; break;

					case '=': which = SyntaxTreeNode::Type::Assign; hasExtension = false; break;
					case ',': which = SyntaxTreeNode::Type::Comma; hasExtension = false; break;

					case '.':
					{
						size_t n = i + 1;

						// The dot operator requires identifiers on both sides	
						if(	parts.back().value->type != TokenType::Identifier ||
							!isToken(TokenType::Identifier, n))
						{
							Logger::error(tokens[i], "'.' requires identifiers on both sides");
							return errorOut();
						}

						which = SyntaxTreeNode::Type::Access;
						hasExtension = false;
						break;
					}

					default:
						Logger::error(tokens[i], "Invalid operator '%c'", *tokens[i].begin);
						return errorOut();
				}
			}

			//	What comes before this operator?
			Token* last = parts.back().value;
			size_t old = i;

			//	Whatever comes before the operator has to be an identifier
			if((which == SyntaxTreeNode::Type::Assign || hasExtension) && last->type != TokenType::Identifier)
			{
				Logger::error(tokens[i], "Expected an identifier before assignment");
				return errorOut();
			}

			//	Does '=' occur after the operator?
			if(hasExtension)
			{
				//	For an example, turn "x += 2" to "x = x + 2" or "x <<= 2" to "x = x << 2"
				parts.push_back({ SyntaxTreeNode::Type::Assign, &tokens[old] });
				parts.push_back({ SyntaxTreeNode::Type::Value, last });

				i = next;
			}

			//	Append the actual operator
			parts.push_back({ which, &tokens[old] });
		}
	}

	if(parts.empty())
	{
		DBG_LOG("Parts empty");
		return true;
	}

	//	TODO If the last part is an operator, make sure that it's post increment/decrement
	if(parts.back().value->type == TokenType::Operator)
	{
		Logger::error(*parts.back().value, "Line cannot end with an operator");
		return errorOut();
	}

	//	We don't want extra "Expression" nodes where brackets start
	if(!inBrackets)
	{
		//	Create a node for the expression that we're about to parse
		current.node->type = SyntaxTreeNode::Type::Expression;
		current.node->left = std::make_shared <SyntaxTreeNode> (current.node, nullptr, SyntaxTreeNode::Type::None);
		current.node = current.node->left.get();
	}

	//	Attempt to parse an expression
	if(!parseExpression(parts, parts.size() - 1, 0, 0, current.node, current))
		return errorOut();

	//	No need to create the next node when inside brackets
	if(!inBrackets)
	{
		//	Now we need a node for whatever comes next. Place it on the right side of the last node
		lastNode->right = std::make_shared <SyntaxTreeNode> (current.node, nullptr, SyntaxTreeNode::Type::None);
		current.node = lastNode->right.get();
	}

	return true;
}

bool Cap::SourceFile::parseLineInBracket(SyntaxTreeNode* node, Token* at, Scope& current)
{
	//	The first position after the opening bracket
	size_t index = tokens.getIndex(at) + 1;

	SyntaxTreeNode* oldNode = current.node;
	current.node = node;

	bool result = parseLine(index, current, true);

	current.node = oldNode;
	return result;
}
