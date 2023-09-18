#include <cap/Type.hh>
#include <cap/Scope.hh>
#include <cap/Function.hh>

#include <cap/node/ExpressionRoot.hh>
#include <cap/node/VariableDeclaration.hh>
#include <cap/node/FunctionDeclaration.hh>

namespace cap
{

Token Scope::consumeName(Tokenizer& tokens)
{
	Token name = tokens.next();

	if(name.getType() != Token::Type::Identifier)
	{
		printf("%lu:%lu Expected an identifier, got '%s'\n", name.getRow(), name.getColumn(), name.getTypeString());
		return Token::createInvalid();
	}

	return name;
}

bool Scope::createFunction(Token&& token, ParserState& state)
{
	state.node = state.node->createNext <FunctionDeclaration> (std::move(token));

	Token name = consumeName(state.tokens);
	scopes.emplace_back(std::make_shared <Function> (*this, std::move(name)));

	auto function = std::static_pointer_cast <Function> (scopes.back());
	ParserState newState(state.tokens, function->getRoot());
	std::static_pointer_cast <FunctionDeclaration> (state.node)->function = function;

	printf("-------------------------- START PARSING FUNCTION --------------------------------------------\n");
	bool ret = function->parse(newState);
	printf("-------------------------- STOP PARSING FUNCTION ---------------------------------------------\n");

	return ret;
}

bool Scope::createType(Token&& token, ParserState& state)
{
	Token name = consumeName(state.tokens);
	scopes.emplace_back(std::make_shared <Type> (*this, std::move(name)));

	BraceMatcher braces;
	return scopes.back()->parse(state);
}

bool Scope::createVariable(ParserState& state)
{
	Token name = consumeName(state.tokens);
	state.node = state.node->createNext <VariableDeclaration> (std::move(name));

	return true;
}

bool Scope::parse(Tokenizer& tokens)
{
	ParserState state(tokens, root);
	return parse(state);
}

bool Scope::parse(ParserState& state)
{
	printf("Scope parse\n");

	// While there are tokens left the brace matcher is tracking braces,
	// consume the token. NOTE: The global scope isn't contained in any
	// braces so there's no brace matching if a scope doesn't have a parent.
	while(!state.tokens.empty() && (!parent || state.braces.depth() > 0))
	{
		Token token = state.tokens.next();
		BraceType braceType = BraceMatcher::getBraceType(token);

		// Is the current token an opening brace?
		if(braceType == BraceType::Opening)
		{
			if(!state.braces.open(std::move(token)))
				return false;
		}

		// Is the current token a closing brace?
		else if(braceType == BraceType::Closing)
		{
			if(!state.braces.close(std::move(token)))
				return false;
		}

		// The current token is not a brace.
		else
		{
			printf("Token %s '%s'\n", token.getTypeString(), token.getString().c_str());

			// "func" indicates that a function should be created.
			if(token == "func")
			{
				// If the function creation fails, stop parsing.
				if(!createFunction(std::move(token), state))
					return false;
			}

			// "type" indicates that a type should be created.
			else if(token == "type")
			{
				// If the type creation fails, stop parsing.
				if(!createType(std::move(token), state))
					return false;
			}

			// "var" indicates that a variable should be created.
			else if(token == "var")
			{
				// If the variable creation fails, stop parsing.
				if(!createVariable(state))
					return false;

				// Since variables require assignment, we can assume that we're in an expression.
				state.inExpression = true;
			}

			else
			{
				if(!state.inExpression)
				{
					state.node = findLastNode();
					printf("CREATE NEXT FOR %u\n", state.node->id);

					state.node = state.node->createNext <ExpressionRoot> (Token::createInvalid());

					state.inExpression = true;
				}

				if(!state.node->handleToken(std::move(token), state))
					return false;
			}
		}
	}

	// If we're not in a global scope and braces are still
	// being tracked, we have unterminated braces.
	if(parent && state.braces.depth() > 0)
	{
		printf("Unterminated brace '%c'\n", state.braces.getMostRecent()[0]);
		return false;
	}

	return true;
}

std::shared_ptr <Node> Scope::findLastNode()
{
	std::shared_ptr <Node> current = root;
	while(current->hasNext())
	{
		current = current->getNext();
	}

	return current;
}

}
