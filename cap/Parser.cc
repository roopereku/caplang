#include <cap/Parser.hh>

#include <cap/event/DebugMessage.hh>
#include <cap/event/ErrorMessage.hh>

#include <cap/node/FunctionDefinition.hh>
#include <cap/node/TypeDefinition.hh>

#include <cassert>

namespace cap
{

static bool isBracket(Token& token)
{
	return token == Token::Type::CurlyBrace ||
			token == Token::Type::Parenthesis ||
			token == Token::Type::SquareBracket;
}

Parser::Parser(Tokenizer& tokens) : tokens(tokens)
{
}

bool Parser::parse(EventEmitter& events, std::shared_ptr <Node> root)
{
	tokens.reset();
	currentNode = root;

	// Iterate through the tokens.
	while(!tokens.empty())
	{
		if(!parseNextToken(events))
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

bool Parser::parseNextToken(EventEmitter& events)
{
	Token token = tokens.next();
	events.emit(DebugMessage("Token: " + token.getString(), token));

	// Handle brackets.
	if(isBracket(token) && !handleBracket(token, events))
	{
		return false;
	}

	// Handle type definitions.
	else if(token == "type" && !parseType(token, events))
	{
		return false;
	}

	// Handle function definitions.
	else if(token == "func" && !parseFunction(token, events))
	{
		return false;
	}

	// Handle variable definitions.
	else if(token == "var" && !parseVariable(token, events))
	{
		return false;
	}

	// TODO: Add statements.

	// Anything else should be an expression.
	else
	{
	}
	
	return true;
}

bool Parser::handleBracket(Token& token, EventEmitter& events)
{
	switch(token[0])
	{
		// Handle opening brackets.
		case '{': case '(': case '[':
		{
			openingBrackets.push(token);
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

bool Parser::parseType(Token& token, EventEmitter& events)
{
	// An identifier is expected after "type".
	auto name = tokens.next();
	if(name.getType() != Token::Type::Identifier)
	{
		events.emit(ErrorMessage("Expected an identifier after type", name));
		return false;
	}

	// Create a type definition node.
	addNode(std::make_shared <TypeDefinition> ());

	return true;
}

bool Parser::parseFunction(Token& token, EventEmitter& events)
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
	if(!handleBracket(opener, events))
	{
		return false;
	}

	// Create a function definition node.
	addNode(std::make_shared <FunctionDefinition> ());

	// While the parameter parenthesis are open, parse the parameters.
	while(openingBrackets.size() > oldOpeners && !tokens.empty())
	{
		if(!parseNextToken(events))
		{
			return false;
		}
	}

	return true;
}

bool Parser::parseVariable(Token& token, EventEmitter& events)
{
	return todo("\"var\" keyword", events);
}

bool Parser::todo(std::string&& msg, EventEmitter& events)
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

}
