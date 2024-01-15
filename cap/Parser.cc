#include <cap/Parser.hh>

#include <cap/event/DebugMessage.hh>
#include <cap/event/ErrorMessage.hh>

#include <cap/node/TypeDefinition.hh>
#include <cap/node/FunctionDefinition.hh>
#include <cap/node/InitializationRoot.hh>
#include <cap/node/ExplicitReturnType.hh>
#include <cap/node/OneSidedOperator.hh>
#include <cap/node/TwoSidedOperator.hh>
#include <cap/node/ReturnStatement.hh>
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

bool Parser::getNextToken(Tokenizer& tokens, Token& result)
{
	result = tokens.next();

	// Make sure that the token is valid.
	if(result == Token::Type::Invalid)
	{
		events.emit(ErrorMessage(std::string(tokens.getErrorString()), result));
		return false;
	}

	// Ignore comments.
	if(result == Token::Type::Comment)
	{
		return getNextToken(tokens, result);
	}

	return true;
}

bool Parser::parse(Tokenizer& tokens, std::shared_ptr <Node> root)
{
	tokens.reset();
	currentNode = root;

	auto token = Token::createInvalid();

	// Iterate through the tokens.
	while(!tokens.empty())
	{
		if(!getNextToken(tokens, token))
		{
			return false;
		}

		else if(!parseToken(token, tokens, true))
		{
			return false;
		}
	}

	// If there is still unterminated opening brackets, show an error.
	if(!openingBrackets.empty())
	{
		return unterminatedBracket(openingBrackets.top());
	}

	// If there's an unfinished expression, finish it.
	if(inExpression && !endExpression(token))
	{
		return false;
	}

	return true;
}

bool Parser::parseToken(Token& token, Tokenizer& tokens, bool breakExpressionOnNewline)
{
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

	// Handle initializations.
	else if(token == "var" || token == "alias")
	{
		if(!parseInitialization(token))
		{
			return false;
		}
	}

	// Handle return statements.
	else if(token == "return")
	{
		if(!parseReturn(token))
		{
			return false;
		}
	}

	// TODO: Add more statements.

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
							if(!currentNode->as <Expression> ()->handleExpressionNode(call, *this))
							{
								return false;
							}

							// The call target is the cached value or the expression of the call operator
							// that became the current node after a call to handleExpressionNode().
							auto target = cachedValue ? std::move(cachedValue) : currentNode->as <OneSidedOperator> ()->stealMostRecentValue();

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

				events.emit(DebugMessage("Begin a subexpression", token));

				// TODO: Handle "{" inside an expression. It could define a tuple.
				if(token[0] == '{')
				{
					return todo("Curly braces inside expressions");
				}

				// Store the old current node and create a temporary one for the subexpression.
				auto oldCurrent = currentNode;
				auto expr = std::make_shared <ExpressionRoot> (token);

				currentNode = expr;
				Token::IndexType currentRow;

				// While the brackets are unterminated, parse tokens.
				while(openingBrackets.size() > oldOpeners && !tokens.empty())
				{
					if(!getNextToken(tokens, token))
					{
						return false;
					}

					currentRow = token.getRow();

					if(!parseToken(token, tokens, false))
					{
						return false;
					}
				}

				// If there are still more opening brackets than at the beginning of the
				// subexpression, show an error message and exit.
				if(openingBrackets.size() > oldOpeners)
				{
					return unterminatedBracket(openingBrackets.top());
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
						bool isError = true;

						// Some expressions support empty brackets.
						if(oldCurrent->type == Node::Type::Expression)
						{
							// Call operator supports empty brackets.
							if(oldCurrent->as <Expression> ()->type == Expression::Type::Operator &&
								oldCurrent->as <Operator> ()->type == Operator::Type::OneSided &&
								oldCurrent->as <OneSidedOperator> ()->type == OneSidedOperator::Type::Call)
							{
								isError = false;
							}
						}

						if(isError)
						{
							events.emit(ErrorMessage("Empty brackets", token));
							return false;
						}
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

			// When a closing curly brace is found and an expression is not active,
			// it indicates a scope closing indicating that the current node should switch to the parent.
			if(!inExpression && token[0] == '}')
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
	if(!getNextToken(tokens, token))
	{
		return false;
	}

	if(token.getType() != Token::Type::Identifier)
	{
		events.emit(ErrorMessage("Expected an identifier after type", token));
		return false;
	}

	// Create a type definition node.
	addNode(std::make_shared <TypeDefinition> (token));

	return true;
}

bool Parser::parseFunction(Token& token, Tokenizer& tokens)
{
	// An identifier is expected after "func".
	if(!getNextToken(tokens, token))
	{
		return false;
	}

	if(token.getType() != Token::Type::Identifier)
	{
		events.emit(ErrorMessage("Expected an identifier after func", token));
		return false;
	}

	// Create a function definition node.
	addNode(std::make_shared <FunctionDefinition> (token));

	if(!getNextToken(tokens, token))
	{
		return false;
	}

	// An opener parenthesis is expected after the function name.
	if(token.getType() != Token::Type::Parenthesis || token[0] != '(')
	{
		events.emit(ErrorMessage("Expected ( after a function name", token));
		return false;
	}

	size_t oldOpeners = openingBrackets.size();

	// Make sure that the opening bracket is valid.
	if(!handleBracketToken(token, tokens))
	{
		return false;
	}

	// After opening the bracket, begin an initialization root for parameters.
	// The order is this in order to prevent erroring out on empty brackets.
	beginExpression(std::make_shared <InitializationRoot> (token, InitializationRoot::Type::Parameter));

	// While the parameter parenthesis are open, parse the parameters.
	while(openingBrackets.size() > oldOpeners && !tokens.empty())
	{
		if(!getNextToken(tokens, token) || !parseToken(token, tokens, false))
		{
			return false;
		}
	}

	// End the parameter initialization expression.
	if(!endExpression(token))
	{
		return false;
	}

	// Make sure that the parent node is the previously added function definition.
	assert(!currentNode->getParent().expired());
	assert(currentNode->getParent().lock()->type == Node::Type::ScopeDefinition);
	assert(currentNode->getParent().lock()->as <ScopeDefinition> ()->type == ScopeDefinition::Type::FunctionDefinition);

	// Switch back to the function definition.
	currentNode = currentNode->getParent().lock();
	bool bodyMissing = true;

	// Look for an explicit return type and the function body beginning.
	while(!tokens.empty())
	{
		if(!getNextToken(tokens, token))
		{
			break;
		}

		// If a curly brace is encountered, stop looking for an explicit return type.
		if(token == Token::Type::CurlyBrace)
		{
			// If an expression was started, try to end it.
			if(inExpression && !endExpression(token))
			{
				return false;
			}

			// Try to open the function body.
			if(!handleBracketToken(token, tokens))
			{
				return false;
			}

			bodyMissing = false;
			break;
		}

		// The token is not a curly brace.
		else
		{
			// If no expression is active, begin one.
			if(!inExpression)
			{
				events.emit(DebugMessage("Initialize an expression for an explicit return type", token));
				beginExpression(std::make_shared <ExplicitReturnType> (token));
			}

			// Try to parse the current token in the context of an expression.
			if(!parseToken(token, tokens, true))
			{
				return false;
			}
		}
	}

	if(bodyMissing)
	{
		events.emit(ErrorMessage("Expected '{' after function declaration", token));
		return false;
	}
	
	return true;
}

bool Parser::parseInitialization(Token& token)
{
	beginExpression(std::make_shared <InitializationRoot> (token));
	return true;
}

bool Parser::parseReturn(Token& token)
{
	beginExpression(std::make_shared <ReturnStatement> (token));
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
		assert(currentNode->as <Expression> ()->type == Expression::Type::Root);
		if(!currentNode->as <ExpressionRoot> ()->handleExpressionNode(std::move(cachedValue), *this))
		{
			return false;
		}
	}

	// Backtrack until the expression root is found.
	while(currentNode->as <Expression> ()->type != Expression::Type::Root)
	{
		currentNode = currentNode->getParent().lock();
	}

	events.emit(DebugMessage(std::string("Stopped at ") + currentNode->getTypeString(), at));
	isPreviousTokenValue = false;
	inExpression = false;

	// If the expression is an initialization, ensure that the syntax is correct.
	// The initialization will be split into separate nodes such as variable definitions.
	if(currentNode->as <ExpressionRoot> ()->type == ExpressionRoot::Type::InitializationRoot)
	{
		auto root = currentNode->as <InitializationRoot> ();

		// If the empty initialization isn't a parameter, throw an error.
		if(root->type != InitializationRoot::Type::Parameter && !root->getRoot())
		{
			events.emit(ErrorMessage("Expected an expression after " + root->token.getString(), root->token));
			return false;
		}

		// If there is an expression within the initialization,
		// try to split it into multiple definition nodes.
		if(root->getRoot() && !ensureInitSyntax(root->getRoot(), root->as <InitializationRoot> ()))
		{
			return false;
		}

		// Remove the initialization root node. Current node should still reference
		// it but it will disappear from the parent scope.
		root->getParent().lock()->removeChildNode(root);
	}

	return true;
}

bool Parser::ensureInitSyntax(std::shared_ptr <Expression> node,
							std::shared_ptr <InitializationRoot> root)
{
	assert(node->type == Expression::Type::Operator);
	bool noAssignment = false;

	// The given operator node should be two sided.
	if(node->as <Operator> ()->type == Operator::Type::TwoSided)
	{
		auto twoSided = node->as <TwoSidedOperator> ();

		switch(twoSided->type)
		{
			// Comma is allowed as it splits multiple definitions.
			case TwoSidedOperator::Type::Comma:
			{
				// Both sides of the comma have to initialize something.
				if(!ensureInitSyntax(twoSided->getLeft(), root) ||
					!ensureInitSyntax(twoSided->getRight(), root))
				{
					return false;
				}

				break;
			}

			// Assignment indicates initialization.
			case TwoSidedOperator::Type::Assignment:
			{
				// The name has to be a value.
				if(twoSided->getLeft()->type != Expression::Type::Value)
				{
					events.emit(ErrorMessage("Cannot apply operators to a name", twoSided->getLeft()->token));
					return false;
				}

				// The name has to be an identifier.
				else if(twoSided->getLeft()->token.getType() != Token::Type::Identifier)
				{
					events.emit(ErrorMessage("Expected an identifier as the name", twoSided->getLeft()->token));
					return false;
				}

				// Make sure that the initialization exists.
				else if(!twoSided->getRight())
				{
					events.emit(ErrorMessage("Expected an initialization after '='", twoSided->token));
					return false;
				}

				addNode(root->createDefinition(
					twoSided->getLeft()->as <Value> (),
					twoSided->getRight()
				));

				break;
			}

			// Any other operator is forbidden as the first node in an initialization.
			default:
			{
				noAssignment = true;
			}
		}
	}

	// The given node isn't a two sided operator.
	else
	{
		noAssignment = true;
	}

	// If there current node doesn't initialize anything, throw an error.
	if(noAssignment)
	{
		events.emit(ErrorMessage("Expected '=' after a name", node->token));
		return false;
	}

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
		if(isPreviousTokenValue)
		{
			events.emit(ErrorMessage("Consecutive values not allowed", token));
			return false;
		}

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

bool Parser::unterminatedBracket(Token at)
{
	events.emit(ErrorMessage(std::string("Unterminated bracket ") + at.getString(), at));
	return false;
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
