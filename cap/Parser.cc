#include <cap/Parser.hh>

#include <cap/event/DebugMessage.hh>
#include <cap/event/ErrorMessage.hh>

#include <cap/node/FunctionDefinition.hh>
#include <cap/node/TypeDefinition.hh>
#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TwoSidedOperator.hh>
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
		if(!parseNextToken(tokens))
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

bool Parser::parseNextToken(Tokenizer& tokens)
{
	Token token = tokens.next();
	events.emit(DebugMessage("Token: " + token.getString(), token));

	// If an expression that's outside brackets is active and the current token
	// is not on the same line where the expression began, end the expression.
	if(inExpression && expressionBraceDepth == 0 && token.getRow() > expressionBeginLine)
	{
		endExpression(token);
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

			// If an expression is active, an opening bracket indicates a subexpression.
			if(inExpression)
			{
				// If an expression is active, keep track of the brace depth;
				expressionBraceDepth++;

				events.emit(DebugMessage("Start a subexpression", token));
				events.emit(DebugMessage("Current is " + currentNode->token.getString(), token));

				// Store the old current node and create a temporary one for the subexpression.
				auto oldCurrent = currentNode;
				currentNode = std::make_shared <Node> ();

				// While the brackets are unterminated, parse tokens.
				while(openingBrackets.size() > oldOpeners && !tokens.empty())
				{
					if(!parseNextToken(tokens))
					{
						return false;
					}
				}

				// If the temporary node is still empty with no cached value, there was nothing inside the brackets.
				if(currentNode->type == Node::Type::Empty)
				{
					// If there was a cached value, give it to the previous current operator.
					if(cachedValue)
					{
						// In nested brackets there could be a case where oldCurrent doesn't point to a
						// real node. In such a case retain the cached value. Example: a = ((5))
						if(oldCurrent->type != Node::Type::Empty)
						{
							events.emit(ErrorMessage("Save cached value from brackets", cachedValue->token));
							oldCurrent->as <Operator> ()->handleValue(std::move(cachedValue));
						}
					}

					else
					{
						events.emit(ErrorMessage("Empty brackets", token));
						return false;
					}
				}

				// If the temporary node isn't empty, it's an expression.
				else
				{
					assert(currentNode->type == Node::Type::Expression);
					assert(oldCurrent->as <Expression> ()->type == Expression::Type::Operator);

					// Treat the subexpression as a value and give it to the outer expression.
					oldCurrent->as <Operator> ()->handleValue(std::move(currentNode->as <Expression> ()));
				}

				currentNode = std::move(oldCurrent);
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

			// If an expression is active, keep track of the brace depth;
			expressionBraceDepth -= inExpression;

			// If the last brace was closed, the expression can now be terminated with
			// a line change. Update the beginning line so that the expression
			// can continue until such an event happens.
			if(inExpression && expressionBraceDepth == 0)
			{
				expressionBeginLine = token.getRow();
			}

			// Close the innermost opener bracket.
			openingBrackets.pop();

			// When a scope ends, switch to the parent of the current node.
			if(token[0] == '}')
			{
				assert(currentNode);
				assert(!currentNode->getParent().expired());

				currentNode = currentNode->getParent().lock();
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
		if(!parseNextToken(tokens))
		{
			return false;
		}
	}

	return true;
}

bool Parser::parseVariable(Token& token, Tokenizer& tokens)
{
	return todo("\"var\" keyword");
}

void Parser::beginExpression(Token& at)
{
	assert(!inExpression);

	events.emit(DebugMessage("Begin an expression", at));
	expressionBeginLine = at.getRow();
	inExpression = true;
}

void Parser::endExpression(Token& at)
{
	assert(inExpression);

	// TODO: Check if cachedValue exists.

	events.emit(DebugMessage("End an expression", at));
	inExpression = false;
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
			auto scope = std::static_pointer_cast <ScopeDefinition> (currentNode);
			currentNode->adopt(node);

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
			currentNode->setNext(std::move(node));
		}
	}

	currentNode = currentNode->getNext();
}

bool Parser::handleExpressionToken(Token& token)
{
	bool addRoot = currentNode->type != Node::Type::Expression;
	bool cacheValue = false;
	std::shared_ptr <Expression> expr;

	if(!inExpression)
	{
		beginExpression(token);
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

		if(!expr)
		{
			events.emit(ErrorMessage("Invalid operator " + token.getString(), token));
			return false;
		}

		expr->setBraceDepth(expressionBraceDepth);

		if(cachedValue)
		{
			events.emit(DebugMessage("Apply cached value", token));
			expr->as <Operator> ()->handleValue(std::move(cachedValue));
		}

		isPreviousTokenValue = false;
	}

	// The token is a value.
	else
	{
		isPreviousTokenValue = true;
		expr = std::make_shared <Value> (token);
		expr->setBraceDepth(expressionBraceDepth);

		// If the value node would become the expression root, cache it instead.
		if(addRoot)
		{
			cacheValue = true;
			addRoot = false;
		}
	}

	// Should the expression value be cached.
	if(cacheValue)
	{
		events.emit(DebugMessage("Cache value " + token.getString(), token));
		cachedValue = std::move(expr);
	}

	// If a root node for the expression should be added, use addNode. "expr" should never be a value.
	else if(addRoot)
	{
		events.emit(DebugMessage("Add expression root", token));
		addNode(std::move(expr));
	}

	// If a the root node of an expression already exists, let the
	// current expression node manipulate the hierarchy of the expression nodes.
	else if(currentNode->type == Node::Type::Expression)
	{
		events.emit(DebugMessage("Call handleExpressionNode", token));
		if(!currentNode->as <Expression> ()->handleExpressionNode(expr, *this))
		{
			return false;
		}
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
