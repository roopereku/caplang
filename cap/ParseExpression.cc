#include "SourceFile.hh"
#include "Logger.hh"
#include "Debug.hh"

bool Cap::SourceFile::errorOut()
{
	valid = false;
	return true;
}

bool Cap::SourceFile::parseExpression(size_t& i, Scope& current, bool inBrackets)
{
	/*	If the first token is an identifier, we could have a
	 *	declaration or an import. In that case we might
	 *	not yet be inside an expression */
	inExpression = !isToken(TokenType::Identifier, i);
	size_t start = i;

	//	Was the last token a double operator such as '++'
	bool lastWasIncDec = false;
	bool lastWasOperator = false;

	bool expectVariableName = false;

	std::vector <ExpressionPart> parts;
	SyntaxTreeNode* lastNode = current.node;

	//	If the current node is "Variable", declare variables implicitly
	if(current.node->type == SyntaxTreeNode::Type::Variable)
	{
		expectVariableName = true;
		inExpression = true;
	}

	//	Create a node for a new expression
	current.node->left = std::make_shared <SyntaxTreeNode> (current.node, &tokens[i], SyntaxTreeNode::Type::None);
	current.node = current.node->left.get();

	for(; i < current.end; i++)
	{
		if(expectVariableName && !isToken(TokenType::Identifier, i))
		{
			Logger::error(tokens.getPath(), tokens[i], "Expected a name for a variable");
			return errorOut();
		}

		//	Did a line change happen?
		if(tokens[i].line > tokens[start].line)
		{
			//	If neither the last or current token is an operator, the expression ends here
			if(!lastWasOperator && !isToken(TokenType::Operator, i))
			{
				i--;
				break;
			}

			//	Update the starting position
			start = i;
		}

		//	Stop if ';' or a closing bracket is encountered
		if(isToken(TokenType::Break, i))
		{
			//	The expression can't be broken inside braces excluding curly braces
			if(inBrackets)
			{
				Logger::error(tokens.getPath(), tokens[i], "Cannot use ';' inside brackets");
				return errorOut();
			}

			break;
		}

		//	Stop the expression if a closing bracket is encountered
		else if(tokens[i].length == 0)
		 	break;

		else if(isToken(TokenType::Identifier, i))
		{
			if(!expectVariableName && i != start && !lastWasOperator)
			{
				Logger::error(tokens.getPath(), tokens[i], "Expected an operator before identifier '%s'", tokens[i].getString().c_str());
				return errorOut();
			}

			size_t old = i;

			bool oldInExpression = inExpression;
			if(parseFunction(i, current))
			{
				//	If we originally weren't in an expression, this is a full function
				if(!oldInExpression) return true;

				//	If parseFunction() failed, stop here
				if(!valid) return true;

				/*	Though this is a little cryptic, store the function index to
				 *	the token's length. This terminates the need for a non-generic
				 *	number in SyntaxTreeNode */
				tokens[old].length = current.getFunctionCount() - 1;
				parts.push_back({ SyntaxTreeNode::Type::AnonFunction, &tokens[old] });
				inExpression = oldInExpression;

				lastWasOperator = false;
				continue;
			}

			//	Are there any declarations or imports
			else if(parseImport(i, current) || parseType(i, current) || parseVariable(i, current))
			{
				//	Does a reserved keyword interrupt the expression?
				if(i == old)
				{
					Logger::error(tokens.getPath(), tokens[i], "Can't use reserved keyword '%s' inside an expression", tokens[i].getString().c_str());
					return errorOut();
				}

				//	Variable mode was started
				else if(i == start + 1)
				{
					expectVariableName = true;
					inExpression = true;
					i--;

					continue;
				}

				/*	An import was added or a type was declared.
				 *	Return to get a clean state for the next expression */
				else return true;
			}

			//	At this point we're definitely inside an expression
			inExpression = true;
			lastWasOperator = false;

			size_t next = i + 1;

			//	TODO handle curly braces
			//	Are there brackets after the identifier
			if((isToken(TokenType::Parenthesis, next) || isToken(TokenType::SquareBracket, next))
				&& tokens[next].length > 0)
			{
				//	Which brackets do we have?
				SyntaxTreeNode::Type t = tokens[next].type == TokenType::Parenthesis ?
					SyntaxTreeNode::Type::Call : SyntaxTreeNode::Type::Subscript;

				//	Add the identifier with a special type and skip over the brackets
				parts.push_back({ t, &tokens[i] });
				i = next + tokens[next].length;
			}

			else
			{
				parts.push_back({ SyntaxTreeNode::Type::Value, &tokens[i] });

				//	If a variable name is expected, use this token as the variable name
				if(expectVariableName)
				{
					current.addVariable(&tokens[i]);
					expectVariableName = false;

					size_t next = i + 1;

					//	If the next token isn't '=', report an error
					if(!isToken(TokenType::Operator, next) ||
						*tokens[next].begin != '=' ||
						tokens[next].length != 1)
					{
						Logger::error(tokens.getPath(), tokens[i], "Expected an initial value or type for variable '%s'", tokens[i].getString().c_str());
						return errorOut();
					}
				}
			}
		}

		//	Check for other values
		else if(!isToken(TokenType::Operator, i))
		{
			if(i > start && !lastWasOperator)
			{
				Logger::error(tokens.getPath(), tokens[i], "Expected an operator before value '%s'", tokens[i].getString().c_str());
				return errorOut();
			}

			lastWasOperator = false;
			parts.push_back({ SyntaxTreeNode::Type::Value, &tokens[i] });

			//	Brackets count as a value
			if(	tokens[i].type == TokenType::Parenthesis ||
				tokens[i].type == TokenType::SquareBracket)
			{
				//	Which type do the brackets have?
				parts.back().type = tokens[i].type == TokenType::Parenthesis ?
					SyntaxTreeNode::Type::Parentheses : SyntaxTreeNode::Type::Array;

				//	Skip the brackets
				i += tokens[i].length;
			}
		}

		//	Check for operators
		else
		{
			size_t next = i + 1;

			/*	An operator can be unary if there's an operator or nothing before it,
			 *	and no operator after it. An unary also cannot happen if the preceding
			 *	operator was '++' or '--' */
			bool possiblyUnary =	!lastWasIncDec &&
									(parts.empty() ||
									parts.back().value->type == TokenType::Operator) &&
									!isToken(TokenType::Operator, next);

			if(!possiblyUnary && lastWasOperator)
			{
				Logger::error(tokens.getPath(), tokens[i], "Expected a value after operator '%s'", parts.back().value->getString().c_str());
				return errorOut();
			}

			SyntaxTreeNode::Type type;
			lastWasOperator = true;

			//	Is the operator '<<', '>>' or '**'
			bool extendableDouble = false;
			lastWasIncDec = false;
			//DBG_LOG("Unary possibly %d", possiblyUnary);

			//	Is the next token the same operator as this one
			if(isToken(TokenType::Operator, next) && *tokens[next].begin == *tokens[i].begin)
			{
				size_t gap = tokens.getBeginIndex(next) - tokens.getBeginIndex(i) - 1;

				if(gap == 0)
				{
					switch(*tokens[next].begin)
					{
						//	TODO Make sure that there's an identifiers on the appropriate side
						case '+': case '-':
						{
							Logger::error(tokens.getPath(), tokens[i], "Unimplemented operator '%c%c'", *tokens[next].begin, *tokens[next].begin);
							return true;

							lastWasIncDec = true;
							lastWasOperator = false;
							break;
						}

						case '<': type = SyntaxTreeNode::Type::BitwiseShiftLeft; extendableDouble = true; break;
						case '>': type = SyntaxTreeNode::Type::BitwiseShiftRight; extendableDouble = true; break;
						case '*': type = SyntaxTreeNode::Type::Power; extendableDouble = true; break;
						case '=': type = SyntaxTreeNode::Type::Equal; break;
						case '&': type = SyntaxTreeNode::Type::And; break;
						case '|': type = SyntaxTreeNode::Type::Or; break;

						case '.': type = SyntaxTreeNode::Type::Range; break;

						default:
							Logger::error(tokens.getPath(), tokens[i], "Invalid operator '%c%c'", *tokens[i].begin, *tokens[i].begin);
							return true;
					}

					i = next;

					//	Stop if this double operator isn't extendable
					if(!extendableDouble)
						continue;

					/*	There might be an operator like '<<=' or '**='.
					 *	Let's go to the normal operator parser and see
					 *	if the current double operator has an extension */
					next = i + 1;
				}
			}

			//	FIXME support multiple unary operators in a row. For an example "!!x"

			/*	If there were no repeating operators and there was the possibility
			 *	of an unary operator, it is an unary operator */
			else if(possiblyUnary)
			{
				//DBG_LOG("Unary '%c'", *tokens[i].begin);

				switch(*tokens[i].begin)
				{
					case '+': type = SyntaxTreeNode::Type::UnaryPositive; break;
					case '-' :type = SyntaxTreeNode::Type::UnaryNegative; break;
					case '@': type = SyntaxTreeNode::Type::Reference; break;
					case '!': type = SyntaxTreeNode::Type::Not; break;
					case '~': type = SyntaxTreeNode::Type::BitwiseNOT; break;

					default:
						Logger::error(tokens.getPath(), tokens[i], "Invalid operator '%c'", *tokens[i].begin);
						return true;
				}

				parts.push_back({ type, &tokens[i] });
				continue;
			}

			//	Is the next token '='
			unsigned char nextEqual =	isToken(TokenType::Operator, next) &&
										*tokens[next].begin == '=';

			bool isComparison = false;

			//	If a double operator is already detected, this switch case is irrelevant
			if(!extendableDouble)
			{
				switch(*tokens[i].begin)
				{
					case '+': type = SyntaxTreeNode::Type::Addition; break;
					case '-': type = SyntaxTreeNode::Type::Subtraction; break;
					case '*': type = SyntaxTreeNode::Type::Multiplication; break;
					case '/': type = SyntaxTreeNode::Type::Division; break;

					case '<': isComparison = true; break;
					case '>': isComparison = true; break;
					case '!': isComparison = true; break;

					case ',':
					{
						if(nextEqual)
						{
							Logger::error(tokens.getPath(), tokens[i], "Invalid operator ',='");
							return true;
						}

						type = SyntaxTreeNode::Type::Comma;

						if(current.node->parent->type == SyntaxTreeNode::Type::Variable)
							expectVariableName = true;

						break;
					}

					case '?':
					{
						if(nextEqual)
						{
							Logger::error(tokens.getPath(), tokens[i], "Invalid operator '?='");
							return true;
						}

						//	TODO implement ternary operators
						Logger::error(tokens.getPath(), tokens[next], "Unimplemented '%c%c'", *tokens[next].begin, *tokens[next].begin);
						return true;
						break;
					}

					case '%': type = SyntaxTreeNode::Type::Modulus; break;

					/*	Since == is handled elsewhere, we have to pretend
					 *	that the next token is '=' to use the same error checking */
					case '=': type = SyntaxTreeNode::Type::Assign; nextEqual = true; break;

					case '^': type = SyntaxTreeNode::Type::BitwiseXOR; break;
					case '&': type = SyntaxTreeNode::Type::BitwiseAND; break;
					case '|': type = SyntaxTreeNode::Type::BitwiseOR; break;

					case '.':
					{
						//	FIXME show the earlier token
						if(i == start || parts.back().value->type != TokenType::Identifier)
						{
							Logger::error(tokens.getPath(), tokens[i], "Expected an identifier before '.'");
							return errorOut();
						}

						else if(!isToken(TokenType::Identifier, next))
						{
							if(next >= tokens.count())
								Logger::error(tokens.getPath(), "Expected an identifier after '.' but got end of file");

							else Logger::error(tokens.getPath(), tokens[next], "Expected an identifier after '.'");

							return errorOut();
						}

						type = SyntaxTreeNode::Type::Access;
						break;
					}

					default:
						Logger::error(tokens.getPath(), tokens[i], "Invalid operator '%c'\n", *tokens[i].begin);
						return errorOut();
				}
			}

			size_t old = i;
			if(nextEqual)
			{
				//	TODO implement "<=", ">=", "!="
				if(isComparison)
				{
					i = next;
				}

				else
				{
					//	Make sure that whatever is on the left is an identifier
					//	FIXME show the earlier token

					//	FIXME allow stuff like "@0x08FC = 12"
					if(i == start || parts.back().value->type != TokenType::Identifier)
					{
						Logger::error(tokens.getPath(), tokens[i], "Expected an identifier before '='");
						return errorOut();
					}

					/*	Because we don't want "x = 2" to turn into "x = x = 2",
					 *	do the expansion only when the operator isn't a single '=' */
					if(*tokens[i].begin != '=')
					{
						//	For an example, turn "x += 2" to "x = x + 2" or "x <<= 2" to "x = x << 2"
						parts.push_back({ SyntaxTreeNode::Type::Assign, &tokens[old] });
						parts.push_back(parts[parts.size() - 2]);

						i = next;
					}
				}
			}

			parts.push_back({ type, &tokens[old] });
		}
	}

	//	Forbid the expression ending with an operator
	if(lastWasOperator)
	{
		Logger::error(tokens.getPath(), tokens[i], "Expressions can't end with operators");
		return errorOut();
	}

	//	We're no longer in an expression
	inExpression = false;

	if(parts.empty())
	{
		//	If there are no expression parts and these are standalone brackets, report an error
		if(	current.node->parent->type == SyntaxTreeNode::Type::Parentheses ||
			current.node->parent->type == SyntaxTreeNode::Type::Array)
		{
			Logger::error(tokens.getPath(), tokens[current.begin], "Empty brackets\n");
			return true;
		}

		DBG_LOG("No parts");
		return true;
	}

	DBG_LOG("Expression parts");
	for(auto& part : parts)
	{
		DBG_LOG("Part '%s' '%s' of type '%s'", SyntaxTreeNode::getTypeString(part.type), part.value->getString().c_str(), part.value->getTypeString());
	}

	//	If inside a type oe the global scope, forbid anything else but variable declarations
	if((current.ctx == ScopeContext::Type || current.parent == nullptr) &&
		current.node->parent->type != SyntaxTreeNode::Type::Variable)
	{
		size_t index = tokens.getIndex(current.node->value);
		Logger::error(tokens.getPath(), tokens[i], "Expected a declaration %s", current.parent ? "inside a type" : "in the global scope");
		return errorOut();
	}

	//	TODO handle brackets
	//	Single values don't need ordering
	if(parts.size() == 1)
	{
		DBG_LOG("Single value");
		current.node->type = SyntaxTreeNode::Type::Value;
		current.node->value = parts[0].value;

		return false;
	}

	//	Add the expression as nodes with the correct precedence
	if(!parseExpressionOrder(parts, parts.size() - 1, 0, 0, current.node, current))
		return true;

	//	Should a node be added for the next expression
	if(!inBrackets)
	{
		//	Create a node for the next expression
		lastNode->right = std::make_shared <SyntaxTreeNode> (lastNode);
		lastNode->right->type = SyntaxTreeNode::Type::Expression;
		current.node = lastNode->right.get();
	}

	return true;
}

bool Cap::SourceFile::parseExpressionOrder(std::vector <ExpressionPart>& parts,
										   size_t offset, size_t end, size_t priority,
										   SyntaxTreeNode* node, Scope& current)
{
	//	Go through each priority
	for(OperatorPrioty ops; !(ops = operatorsAtPriority(priority)).empty(); priority++)
	{
		//	Go through the parts from right to left
		for(size_t i = offset; end == 0 ? (i < parts.size()) : (i >= end); i--)
		{
			//	If the operator/value is irrelevant, skip it
			if(!ops.contains(parts[i].type) || parts[i].used)
				continue;

			/*	Because of the way we order the expression, stuff like "x = i = 3" is
			 *	parsed as (x = i) = 3 which is wrong. If there are 2 consecutive assignments,
			 *	ignore the first one so that the expression is parsed as x = (i = 3) */
			if(	priority == 1 && (end == 0 ? (i - 2 < parts.size()) : (i - 2 >= end)) &&
				!parts[i - 2].used && ops.contains(parts[i - 2].type))
			{
				i--;
				continue;
			}

			node->type = parts[i].type;
			node->value = parts[i].value;
			parts[i].used = true;
			
			int m = -1;
			bool pickNext = false;

			/*	If there's an unary operator, we want to use whatever is immediately
			 *	on the right as the value. Ignore the left side in the next for loop
			 *	and tell it to use the left node */
			if(	node->type >= SyntaxTreeNode::Type::Not &&
				node->type <= SyntaxTreeNode::Type::UnaryNegative)
			{
				pickNext = true;
				m = 1;
			}

			for(; m <= 1; m+=2)
			{
				//	Which side are we checking
				std::shared_ptr <SyntaxTreeNode>& side = m < 0 || pickNext ? node->left : node->right;

				/*	If there's no operator after the next/previous token, or it is used,
				 *	we can safely use the next/previous token as a value */
				if(pickNext || i + (m * 2) >= parts.size() || parts[i + (m * 2)].used)
				{
					//	Is the value contained in brackets
					if( parts[i + m].value->type == TokenType::Parenthesis ||
						parts[i + m].value->type == TokenType::SquareBracket)
					{
						//	Parse the contents of the brackets
						side = std::make_shared <SyntaxTreeNode> (node, parts[i + m].value, parts[i + m].type);
						if(!parseExpressionInBracket(side.get(), parts[i + m].value, current))
							return false;
					}

					//	The value isn't inside brackets so just use it as it is
					else
					{
						//	Create a new node for the value
						side = std::make_shared <SyntaxTreeNode> (node, parts[i + m].value, parts[i + m].type);

						//	Is the value actually a call or a subscript
						if(	parts[i + m].type == SyntaxTreeNode::Type::Call ||
							parts[i + m].type == SyntaxTreeNode::Type::Subscript)
						{
							//	Because the brackets are in the next token, find the next usable token
							size_t tokenIndex = tokens.getIndex(parts[i + m].value) + 1;
							skipComments(tokenIndex);

							//	Create a new expression node for what's inside the brackets and parse the contents
							side->left = std::make_shared <SyntaxTreeNode> (node, &tokens[tokenIndex], SyntaxTreeNode::Type::Expression);
							if(!parseExpressionInBracket(side->left.get(), side->left->value, current))
								return false;
						}
					}

					//	Mark the value used
					parts[i + m].used = true;
				}

				//	The operator after the value takes precedence so we need to parse it first
				else
				{
					//	Determine appropriate offsets and ends depending on which side we are on
					size_t newOffset = m < 0 ? i - 1 : offset;
					size_t newEnd = m < 0 ? 0 : i;

					//	Initialize the left/right branch and recursively fill it
					side = std::make_shared <SyntaxTreeNode> (node);
					if(!parseExpressionOrder(parts, newOffset, newEnd, priority, side.get(), current))
						return false;
				}
			}
		}
	}

	return true;
}

bool Cap::SourceFile::parseExpressionInBracket(SyntaxTreeNode* node, Token* at, Scope& current)
{
	//	Where is the token at the beginning of the brackets
	size_t index = tokens.getIndex(at) + 1;

	//	Save old states
	size_t oldEnd = current.end;
	size_t oldBegin = current.begin;
	SyntaxTreeNode* oldCurrentNode = current.node;

	//	Temporarily modify the scope to fit the brackets and parse the contents
	current.node = node;
	current.end = index + at->length;
	current.begin = index - 1;
	bool result = parseExpression(index, current, true);

	//	Restore the earlier state
	current.node = oldCurrentNode;
	current.begin = oldBegin;
	current.end = oldEnd;

	return result;
}

Cap::OperatorPrioty Cap::operatorsAtPriority(size_t priority)
{
	using T = SyntaxTreeNode::Type;

	switch(priority)
	{
		case 0: return OperatorPrioty(T::Comma);
		case 1: return OperatorPrioty(T::Assign);
		case 2: return OperatorPrioty(T::BitwiseOR);
		case 3: return OperatorPrioty(T::BitwiseAND);
		case 4: return OperatorPrioty(T::BitwiseXOR);
		case 5: return OperatorPrioty(T::Equal, T::Inequal);
		case 6: return OperatorPrioty(T::Less, T::Greater, T::LessEqual, T::GreaterEqual);
		case 7: return OperatorPrioty(T::BitwiseShiftLeft, T::BitwiseShiftRight);
		case 8: return OperatorPrioty(T::Addition, T::Subtraction);
		case 9: return OperatorPrioty(T::Multiplication, T::Division, T::Modulus);
		case 10: return OperatorPrioty(T::Power);
		case 11: return OperatorPrioty(T::UnaryPositive, T::UnaryNegative, T::Not, T::BitwiseNOT, T::Reference);
		case 12: return OperatorPrioty(T::Call, T::Subscript, T::Access);
	}

	return OperatorPrioty();
}
