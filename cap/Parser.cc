#include <cap/Parser.hh>

#include <cap/event/DebugMessage.hh>
#include <cap/event/ErrorMessage.hh>

#include <cap/node/TypeDefinition.hh>
#include <cap/node/FunctionDefinition.hh>
#include <cap/node/VariableDefinition.hh>
#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/ExpressionRoot.hh>
#include <cap/node/CallOperator.hh>
#include <cap/node/Value.hh>

#include <cassert>

namespace cap
{

static bool isBracket(Token& token)
{
	return token == Token::Type::CurlyBrace ||
			token == Token::Type::Parenthesis ||
			token == Token::Type::SquareBracket;
}

Parser::Parser(EventEmitter& events) : events(events)
{
}

bool Parser::parse(Tokenizer& tokens, std::shared_ptr <Node> root)
{
	tokens.reset();
	currentNode = root;

	// Iterate through the tokens.
	while(!tokens.empty())
	{
		Token token = tokens.next();

		// Make sure that the token is valid.
		if(token == Token::Type::Invalid)
		{
			events.emit(ErrorMessage("Error in '" + token.getString() + "': " + std::string(tokens.getErrorString()), token));
			return false;
		}

		if(!parseToken(token, tokens, true))
		{
			return false;
		}
	}

	// If there is still unterminated opening brackets, show an error.
	if(!openingBrackets.empty())
	{
		auto& at = openingBrackets.top();
		events.emit(ErrorMessage(std::string("Unterminated opening bracket ") + at.getString(), at));

		return false;
	}

	return true;
}

bool Parser::parseToken(Token& token, Tokenizer& tokens, bool breakExpressionOnNewline)
{
	events.emit(DebugMessage("Token: " + token.getString(), token));

	// If an expression that's outside brackets is active and the current token
	// is not on the same line where the expression began, end the expression.
	if(inExpression && breakExpressionOnNewline && token.getRow() > expressionBeginLine)
	{
		if(!endExpression(token))
		{
			return false;
		}
	}

	// Handle brackets.
	if(isBracket(token))
	{
		if(!handleBracketToken(token, tokens))
		{
			return false;
		}
	}

	// Handle type definitions.
	else if(token == "type")
	{
		if(!parseType(token, tokens))
		{
			return false;
		}
	}

	// Handle function definitions.
	else if(token == "func")
	{
		if(!parseFunction(token, tokens))
		{
			return false;
		}
	}

	// Handle variable definitions.
	else if(token == "var")
	{
		if(!parseVariable(token, tokens))
		{
			return false;
		}
	}

	// TODO: Add statements.

	// Anything else should be an expression.
	else if(!handleExpressionToken(token))
	{
		return false;
	}
	
	return true;
}

bool Parser::handleBracketToken(Token& token, Tokenizer& tokens)
{
	switch(token[0])
	{
		// Handle opening brackets.
		case '{': case '(': case '[':
		{
			size_t oldOpeners = openingBrackets.size();
			openingBrackets.push(token);

			// TODO: If '(' or '[' is encountered, start an expression.

			// If an expression is active, an opening bracket indicates a subexpression.
			if(inExpression)
			{
				// If the previous token was a value, brackets have a difference meaning.
				if(isPreviousTokenValue)
				{
					switch(token[0])
					{
						// "(" indicates a function call.
						case '(':
						{
							auto call = std::make_shared <CallOperator> (token);

							events.emit(DebugMessage(std::string(currentNode->getTypeString()) + " handles call operator", token));
							if(!currentNode->as <Expression> ()->handleExpressionNode(call, *this))
							{
								return false;
							}

							// The call target is the cached value or the expression of the call operator
							// that became the current node after a call to handleExpressionNode().
							auto target = cachedValue ? cachedValue : currentNode->as <OneSidedOperator> ()->stealMostRecentValue();

							call->setTarget(std::move(target));
							assert(call->getTarget());

							break;
						}

						default:
						{
							return todo("Implement brace " + token.getString() + " after a value");
						}
					}

					// The subexpression might get confused if the previous token is a value.
					isPreviousTokenValue = false;
				}

				events.emit(DebugMessage("Start a subexpression", token));

				// Store the old current node and create a temporary one for the subexpression.
				auto oldCurrent = currentNode;
				auto expr = std::make_shared <ExpressionRoot> (token);

				currentNode = expr;
				Token::IndexType currentRow;

				// While the brackets are unterminated, parse tokens.
				while(openingBrackets.size() > oldOpeners && !tokens.empty())
				{
					Token token = tokens.next();
					currentRow = token.getRow();

					if(!parseToken(token, tokens, false))
					{
						return false;
					}
				}

				// In order to continue the expression outside brackets, update the beginning line.
				expressionBeginLine = currentRow;
				events.emit(DebugMessage("End a subexpression. Expression can continue on line " + std::to_string(expressionBeginLine), token));

				bool handleCachedInOld = false;
				assert(currentNode->type == Node::Type::Expression);

				// If the current node is still a root, the brackets could be empty.
				if(currentNode->as <Expression> ()->type == Expression::Type::Root)
				{
					// If there is a cached value, the brackets aren't empty.
					if(cachedValue)
					{
						// Only if the previous current node was an expression, apply the cached value.
						// Otherwise the cached value is retained for further use.
						//
						// In a case such as "a = ((5))" the previous current node would be an empty node
						// when the inner subexpression exits.
						if(oldCurrent->type == Node::Type::Expression)
						{
							handleCachedInOld = true;
						}
					}

					else
					{
						events.emit(ErrorMessage("Empty brackets", token));
						return false;
					}
				}

				// If there is no cached value, cache the expression.
				if(!cachedValue)
				{
					// In order to use the current expression in outer expressions, cache it.
					events.emit(DebugMessage("Cache the current expression", token));
					cachedValue = std::move(currentNode->as <Expression> ());
				}

				// If the previous current node is an expression, try to give it the cached value.
				if(oldCurrent->type == Node::Type::Expression)
				{
					handleCachedInOld = true;
				}

				// Should the cached value be handled in the old current node.
				if(handleCachedInOld)
				{
					// If the old current node isn't a root, it should be an operator which
					// can handle the cached value.
					if(oldCurrent->as <Expression> ()->type != Expression::Type::Root)
					{
						assert(oldCurrent->as <Expression> ()->type == Expression::Type::Operator);

						events.emit(DebugMessage("Apply the cached value from a subexpression", token));
						oldCurrent->as <Operator> ()->handleValue(std::move(cachedValue));
					}
				}

				// Switch back to the previous current node.
				currentNode = std::move(oldCurrent);

				// Braces are treated as values.
				isPreviousTokenValue = true;
			}

			break;
		}

		// Handle closing brackets.
		case '}': case ')': case ']':
		{
			// If the opening bracket isn't of the same type as the closer, report a mismatch.
			auto& opener = openingBrackets.top();
			if(opener.getType() != token.getType())
			{
				events.emit(ErrorMessage(
					std::string("Mismatching brackets ") + opener.getString() + " and " + token.getString(),
					token)
				);

				return false;
			}

			// Close the innermost opener bracket.
			openingBrackets.pop();

			// When a scope ends, switch to the parent scope.
			if(token[0] == '}')
			{
				// Make sure that the current node is the current scope.
				if(currentNode->type != Node::Type::ScopeDefinition)
				{
					currentNode = currentNode->getParent().lock();
					assert(currentNode->type == Node::Type::ScopeDefinition);
				}

				assert(currentNode);
				assert(!currentNode->getParent().expired());

				currentNode = currentNode->getParent().lock();
				assert(currentNode->type == Node::Type::ScopeDefinition);

				events.emit(DebugMessage("Switch to outer scope " + currentNode->as <ScopeDefinition> ()->name.getString(), token));
			}
		}
	}

	return true;
}

bool Parser::parseType(Token& token, Tokenizer& tokens)
{
	// An identifier is expected after "type".
	auto name = tokens.next();
	if(name.getType() != Token::Type::Identifier)
	{
		events.emit(ErrorMessage("Expected an identifier after type", name));
		return false;
	}

	// Create a type definition node.
	addNode(std::make_shared <TypeDefinition> (name));

	return true;
}

bool Parser::parseFunction(Token& token, Tokenizer& tokens)
{
	// An identifier is expected after "func".
	auto name = tokens.next();
	if(name.getType() != Token::Type::Identifier)
	{
		events.emit(ErrorMessage("Expected an identifier after func", name));
		return false;
	}

	// An opener parenthesis is expected after the function name.
	auto opener = tokens.next();
	if(opener.getType() != Token::Type::Parenthesis || opener[0] != '(')
	{
		events.emit(ErrorMessage("Expected ( after a function name", opener));
		return false;
	}

	size_t oldOpeners = openingBrackets.size();

	// Make sure that the opening bracket is valid.
	if(!handleBracketToken(opener, tokens))
	{
		return false;
	}

	// Create a function definition node.
	addNode(std::make_shared <FunctionDefinition> (name));

	// While the parameter parenthesis are open, parse the parameters.
	while(openingBrackets.size() > oldOpeners && !tokens.empty())
	{
		Token token = tokens.next();
		if(!parseToken(token, tokens, false))
		{
			return false;
		}
	}

	return true;
}

bool Parser::parseVariable(Token& token, Tokenizer& tokens)
{
	beginExpression(std::make_shared <VariableDefinition> (token));
	return true;
}

void Parser::beginExpression(std::shared_ptr <Expression>&& root)
{
	assert(!inExpression);

	events.emit(DebugMessage("Begin an expression", root->token));
	expressionBeginLine = root->token.getRow();
	inExpression = true;

	addNode(std::move(root));
}

bool Parser::endExpression(Token& at)
{
	assert(inExpression);
	events.emit(DebugMessage("End an expression", at));

	// If there still is a cached value, apply it.
	if(cachedValue)
	{
		return todo("Handle cachedValue before ending the expression");
	}

	// Backtrack until the expression root is found.
	while(currentNode->as <Expression> ()->type != Expression::Type::Root)
	{
		currentNode = currentNode->getParent().lock();
	}

	events.emit(DebugMessage(std::string("Stopped at ") + currentNode->getTypeString(), at));

	inExpression = false;
	return true;
}

bool Parser::todo(std::string&& msg)
{
	events.emit(ErrorMessage("TODO: " + std::move(msg), Token::createInvalid()));
	return false;
}

void Parser::addNode(std::shared_ptr <Node>&& node)
{
	switch(currentNode->type)
	{
		// If the current node is a scope, add the new node under it.
		case Node::Type::ScopeDefinition:
		{
			// Make sure that the current scope node is the parent of the new node.
			auto scope = currentNode-> as <ScopeDefinition> ();

			scope->adopt(node);

			// If not scope root is set, initialize it.
			if(!scope->getRoot())
			{
				scope->initializeRoot(std::move(node));
				currentNode = scope->getRoot();

				// Since there will be no next node, return here
				// to skip the reassignment.
				return;
			}

			// The scope root exists so put the new node after the last
			// node inside the scope.
			else
			{
				currentNode = scope->getRoot()->findLast();
				currentNode->setNext(std::move(node));
			}

			break;
		}

		default:
		{
			assert(!currentNode->getParent().expired());
			currentNode->getParent().lock()->adopt(node);

			currentNode->setNext(std::move(node));
		}
	}

	currentNode = currentNode->getNext();
}

bool Parser::handleExpressionToken(Token& token)
{
	std::shared_ptr <Expression> expr;

	if(!inExpression)
	{
		beginExpression(std::make_shared <ExpressionRoot> (token));
	}

	if(token.getType() == Token::Type::Operator)
	{
		// If the previous token wasn't a value, assume this to be an unary operator.
		if(!isPreviousTokenValue)
		{
			expr = OneSidedOperator::parseToken(token);
		}

		// This is not an unary operator.
		else
		{
			expr = TwoSidedOperator::parseToken(token);
		}

		isPreviousTokenValue = false;

		if(!expr)
		{
			events.emit(ErrorMessage("Invalid operator " + token.getString(), token));
			return false;
		}

		if(cachedValue)
		{
			events.emit(DebugMessage("Apply cached value", token));
			expr->as <Operator> ()->handleValue(std::move(cachedValue));
		}
	}

	// The token is a value.
	else
	{
		isPreviousTokenValue = true;
		expr = std::make_shared <Value> (token);

		// If the value node would become the expression root node, cache it instead.
		if(currentNode->as <Expression> ()->type == Expression::Type::Root)
		{
			events.emit(DebugMessage("Cache value " + token.getString(), token));
			cachedValue = std::move(expr);
		}

		// The current node isn't the root and should be able to handle the value.
		else
		{
			events.emit(DebugMessage("Handle value " + token.getString() + " in " + currentNode->getTypeString(), token));

			assert(currentNode->as <Expression> ()->type == Expression::Type::Operator);
			if(!currentNode->as <Operator> ()->handleValue(std::move(expr)))
			{
				return false;
			}
		}

		return true;
	}

	// Let the current expression node manipulate the hierarchy of the expression nodes.
	events.emit(DebugMessage("Call handleExpressionNode", token));
	if(!currentNode->as <Expression> ()->handleExpressionNode(expr, *this))
	{
		return false;
	}

	return true;
}

void Parser::setCurrentNode(std::shared_ptr <Node> node)
{
	currentNode = std::move(node);
}

std::shared_ptr <Node> Parser::getCurrentNode()
{
	return currentNode;
}

}
