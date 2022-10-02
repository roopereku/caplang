#include "SourceFile.hh"
#include "Logger.hh"
#include "Debug.hh"

bool Cap::SourceFile::parseExpression(std::vector <ExpressionPart>& parts,
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
						if(!parseLineInBracket(side.get(), parts[i + m].value, current))
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
							if(!parseLineInBracket(side->left.get(), side->left->value, current))
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
					if(!parseExpression(parts, newOffset, newEnd, priority, side.get(), current))
						return false;
				}
			}
		}
	}

	return true;
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
