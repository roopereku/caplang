#include "SourceFile.hh"
#include "Debug.hh"

bool Cap::SourceFile::parseExpression(size_t& i, Scope& current)
{
	DBG_LOG("Parsing expression in scope %lu - %lu", current.begin, current.end);

	/*	If the first token is an identifier, we could have a
	 *	declaration or an import. In that case we might
	 *	not yet be inside an expression */
	inExpression = !isToken(TokenType::Identifier, i);
	size_t start = i;

	//	Was the last token a double operator such as '++'
	bool lastWasIncDec = false;
	bool lastWasOperator = false;

	DBG_LOG("Current node is of type '%s'", current.node->getTypeString());

	//	TODO use parts.back() instead of tokens[i - 1]
	std::vector <ExpressionPart> parts;
	SyntaxTreeNode* lastNode = current.node;

	//	Create a node for a new expression
	current.node->left = std::make_shared <SyntaxTreeNode> (current.node, &tokens[i], SyntaxTreeNode::Type::None);
	current.node = current.node->left.get();

	for(; i < current.end; i++)
	{
		//	Line change breaks the loop if the last tokens wasn't an operator
		if(tokens[i].line > tokens[start].line && !lastWasOperator)
		{
			DBG_LOG("Line change", "");
			i--;
			break;
		}

		else if(tokens[i].type == TokenType::Break)
			break;

		else if(tokens[i].type == TokenType::Identifier)
		{
			if(i != start && !lastWasOperator)
				return showExpected("an operator before identifier", i);

			size_t old = i;

			//	Are there any declarations or imports
			if(parseImport(i, current) || parseType(i, current) ||
				parseVariable(i, current) || parseFunction(i, current))
			{
				//	If something interrupts an expression, error out
				if(old > start)
					return showExpected("';' or a new line", old);

				/*	There was a valid declaration or import.
				 *	Let's exit the function to get a clean state */

				//	Variable declarations shouldn't cause a return
				if(i == start + 1)
				{
					DBG_LOG("Variable '%s' was declared", tokens[i].getString().c_str());

					//	Variable declaration can still go wrong
					if(!valid) return true;
				}

				else return true;
			}

			//	At this point we're definitely inside an expression
			inExpression = true;
			lastWasOperator = false;

			SyntaxTreeNode::Type t = SyntaxTreeNode::Type::Value;
			size_t next = i + 1;

			//	Stuff like id() is treated as a call
			if(isToken(TokenType::Parenthesis, next))
			{
				DBG_LOG("Expression: Call '%s'", tokens[i].getString().c_str());
				parts.push_back({ SyntaxTreeNode::Type::Call, &tokens[i] });
				i += tokens[i].length + 1;
			}

			else
			{
				DBG_LOG("Expression: Identifier '%s'", tokens[i].getString().c_str());
				parts.push_back({ SyntaxTreeNode::Type::Value, &tokens[i] });
			}
		}

		//	Check for other values
		else if(tokens[i].type != TokenType::Operator)
		{
			if(i > start && !lastWasOperator)
				return showExpected("an operator before value", i);

			DBG_LOG("Expression: %s '%s'", tokens[i].getTypeString(), tokens[i].getString().c_str());
			lastWasOperator = false;
			parts.push_back({ SyntaxTreeNode::Type::Value, &tokens[i] });

			if(tokens[i].type == TokenType::Parenthesis)
			{
				DBG_LOG("Parenthesis at %lu - %lu", i, i + tokens[i].length);
				i += tokens[i].length + 1;
			}
		}

		//	Check for operators
		else
		{
			size_t next = i + 1;
			DBG_LOG("Expression: Operator '%s'", tokens[i].getString().c_str());

			/*	An operator can be an unary operator if there's an operator before it,
			 *	and no operator after it. An unary also cannot happen if the preceding
			 *	operator was '++' or '--' */
			bool possiblyUnary =	!lastWasIncDec &&
									(i == start ||
									tokens[i - 1].type == TokenType::Operator) &&
									!isToken(TokenType::Operator, next);
			if(!possiblyUnary && lastWasOperator)
				return showExpected("a value after an operator", i);

			SyntaxTreeNode::Type type;
			lastWasOperator = true;

			//	Is the operator '<<', '>>' or '**'
			bool extendableDouble = false;
			lastWasIncDec = false;
			//DBG_LOG("Unary possibly %d", possiblyUnary);

			//	Is the next token the same operator as this one
			if(isToken(TokenType::Operator, next) && *tokens[next].begin == *tokens[i].begin)
			{
				//	FIXME When not in parenthesis, check if the line changes
				size_t gap = tokens.getBeginIndex(next) - tokens.getBeginIndex(i) - 1;
				//DBG_LOG("Gap is %lu", gap);

				if(gap == 0)
				{
					DBG_LOG("Combine '%c'", *tokens[next].begin);

					switch(*tokens[next].begin)
					{
						//	TODO Make sure that there's an identifiers on the appropriate side
						case '+': case '-':
						{
							ERROR_LOG(tokens[next], "Unimplemented '%c%c'", *tokens[next].begin, *tokens[next].begin);
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
							ERROR_LOG(tokens[i], "Invalid operator '%c%c'\n", *tokens[i].begin, *tokens[i].begin);
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

					DBG_LOG("Checking for extension for double '%c'", *tokens[i].begin);
				}
			}

			//	FIXME support multiple unary operators in a row. For an example "!!x"

			/*	If there were no repeating operators and there was the possibility
			 *	of an unary operator, it is an unary operator */
			else if(possiblyUnary)
			{
				DBG_LOG("Unary '%c'", *tokens[i].begin);

				switch(*tokens[i].begin)
				{
					case '+': type = SyntaxTreeNode::Type::UnaryPositive; break;
					case '-' :type = SyntaxTreeNode::Type::UnaryNegative; break;
					case '@': type = SyntaxTreeNode::Type::Reference; break;
					case '!': type = SyntaxTreeNode::Type::Not; break;
					case '~': type = SyntaxTreeNode::Type::BitwiseNOT; break;

					default:
						ERROR_LOG(tokens[i], "Invalid unary operator '%c'\n", *tokens[i].begin);
						return true;
				}

				parts.push_back({ type, &tokens[i] });
				continue;
			}

			DBG_LOG("Normal operator '%c'", *tokens[i].begin);
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
							ERROR_LOG(tokens[i], "Invalid operator ',='");
							return true;
						}

						type = SyntaxTreeNode::Type::Comma;
						break;
					}

					case '?':
					{
						if(nextEqual)
						{
							ERROR_LOG(tokens[i], "Invalid operator '?='");
							return true;
						}

						//	TODO implement ternary operators
						ERROR_LOG(tokens[next], "Unimplemented '%c%c'", *tokens[next].begin, *tokens[next].begin);
						return true;
						break;
					}

					case '%': break;

					/*	Since == is handled elsewhere, we have to pretend
					 *	that the next token is '=' to use the same error checking */
					case '=': type = SyntaxTreeNode::Type::Assign; nextEqual = true; break;

					case '^': type = SyntaxTreeNode::Type::BitwiseXOR; break;
					case '&': type = SyntaxTreeNode::Type::BitwiseAND; break;
					case '|': type = SyntaxTreeNode::Type::BitwiseOR; break;

					case '.':
					{
						//	FIXME show the earlier token
						if(i == start || tokens[i - 1].type != TokenType::Identifier)
							return showExpected("an identifier before '.'", i);

						else if(!isToken(TokenType::Identifier, next))
							return showExpected("an identifier after '.'", next);

						type = SyntaxTreeNode::Type::Access;
						break;
					}

					default:
						ERROR_LOG(tokens[i], "Invalid operator '%c'\n", *tokens[i].begin);
						return true;
				}
			}

			size_t old = i;
			if(nextEqual)
			{
				if(isComparison)
				{
					i = next;
				}

				else
				{
					//	Make sure that whatever is on the left is an identifier
					//	FIXME show the earlier token
					if(i == start || parts.back().value->type != TokenType::Identifier)
						return showExpected("an identifier before assignment", i);

					//	Do nothing if the part is assignment
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
		return showExpected("a value after an operator", i);

	inExpression = false;

	if(parts.empty())
	{
		DBG_LOG("No parts%s", "");
		return false;
	}

	DBG_LOG("Expression parts %s", "");
	for(auto& part : parts)
		DBG_LOG("Part '%s' '%s' of type '%s'", SyntaxTreeNode::getTypeString(part.type), part.value->getString().c_str(), part.value->getTypeString());

	//	If inside a type oe the global scope, forbid anything else but variable declarations
	if((current.ctx == ScopeContext::Type || current.parent == nullptr) &&
		current.node->parent->type != SyntaxTreeNode::Type::Variable)
	{
		size_t index = tokens.getIndex(current.node->value);
		return showExpected(std::string("a declaration ") + (current.parent ? "inside a type" : "in the global scope"), index);
	}

	//	Variables always require a type or an initial value
	if(current.node->parent->type == SyntaxTreeNode::Type::Variable)
	{
		size_t index = parts.size() < 2 ? 0 : 1;
		if(parts[index].type != SyntaxTreeNode::Type::Assign)
		{
			index = tokens.getIndex(parts[index].value);
			return showExpected("a type or an initial value for variable", index);
		}
	}

	//	Single values don't need ordering
	if(parts.size() == 1)
	{
		DBG_LOG("Single value%s", "");
		current.node->type = SyntaxTreeNode::Type::Value;
		current.node->value = parts[0].value;

		return false;
	}

	//	Add the expression as nodes with the correct precedence
	parseExpressionOrder(parts, parts.size() - 1, 0, 0, current.node);

	//	Create a node for the next expression
	lastNode->right = std::make_shared <SyntaxTreeNode> (lastNode);
	lastNode->right->type = SyntaxTreeNode::Type::Expression;
	current.node = lastNode->right.get();

	DBG_LOG("End of expression. Current type '%s', branches from '%s'", current.node->getTypeString(), current.node->parent->getTypeString());

	return true;
}

void Cap::SourceFile::parseExpressionOrder(std::vector <ExpressionPart>& parts, size_t offset,
										   size_t end, size_t priority, SyntaxTreeNode* current)
{
	//	Uncomment if something goes horribly wrong :-)
	//priority = 0;

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
			if(	priority == 1 && end == 0 ? (i - 2 < parts.size()) : (i - 2 >= end) &&
				!parts[i - 2].used &&  ops.contains(parts[i - 2].type))
			{
				i--;
				continue;
			}

			current->type = parts[i].type;
			current->value = parts[i].value;
			parts[i].used = true;

			DBG_LOG("Primary operator is '%s'.", current->getTypeString());

			//	Unary operators only use the right hand side value
			if(	current->type >= SyntaxTreeNode::Type::Not &&
				current->type <= SyntaxTreeNode::Type::UnaryNegative)
			{
				current->right = std::make_shared <SyntaxTreeNode> (current, parts[i + 1].value, SyntaxTreeNode::Type::Value);
				parts[i + 1].used = true;
				continue;
			}

			if(current->parent != nullptr)
				DBG_LOG("Branches from '%s'", current->parent->getTypeString());

			//	FIXME Make the code below a single function and call it twice

			//	The value on the left side is used if no unused operator is before it
			if(i - 2 >= parts.size() || parts[i - 2].used)
			{
				current->left = std::make_shared <SyntaxTreeNode> (current, parts[i - 1].value, parts[i - 1].type);
				DBG_LOG("Value on the left is '%s'", current->left->value->getString().c_str());
				parts[i - 1].used = true;
			}

			else
			{
				//	Initialize the left branch and recursively fill it
				current->left = std::make_shared <SyntaxTreeNode> (current);
				DBG_LOG("Parsing operator on the left of '%s", current->getTypeString());
				parseExpressionOrder(parts, i - 1, 0, priority, current->left.get());
			}

			//	The value on the right side is used if no unused operator is after it
			if(i + 2 >= parts.size() || parts[i + 2].used)
			{
				current->right = std::make_shared <SyntaxTreeNode> (current, parts[i + 1].value, parts[i + 1].type);
				DBG_LOG("Value on the right is '%s'", current->right->value->getString().c_str());
				parts[i + 1].used = true;
			}

			else
			{
				//	Initialize the right branch and recursively fill it
				current->right = std::make_shared <SyntaxTreeNode> (current);
				DBG_LOG("Parsing operator on the right of '%s'", current->getTypeString());
				parseExpressionOrder(parts, offset, i, priority, current->right.get());
			}
		}
	}
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

		case 12: return OperatorPrioty(T::Call, T::Access);
	}

	return OperatorPrioty();
}
