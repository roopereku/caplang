#include <cap/Type.hh>
#include <cap/Scope.hh>
#include <cap/Function.hh>

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

bool Scope::createFunction(Tokenizer& tokens)
{
	Token name = consumeName(tokens);
	scopes.emplace_back(std::make_shared <Function> (*this, std::move(name)));

	BraceMatcher braces;
	return scopes.back()->parse(tokens, braces);
}

bool Scope::createType(Tokenizer& tokens)
{
	Token name = consumeName(tokens);
	scopes.emplace_back(std::make_shared <Type> (*this, std::move(name)));

	BraceMatcher braces;
	return scopes.back()->parse(tokens, braces);
}

bool Scope::parse(Tokenizer& tokens, BraceMatcher& braces)
{
	printf("Scope parse\n");

	// While there are tokens left the brace matcher is tracking braces,
	// consume the token. NOTE: The global scope isn't contained in any
	// braces so there's no brace matching if a scope doesn't have a parent.
	while(!tokens.empty() && (!parent || braces.depth() > 0))
	{
		Token token = tokens.next();
		BraceType braceType = BraceMatcher::getBraceType(token);

		// Is the current token an opening brace?
		if(braceType == BraceType::Opening)
		{
			if(!braces.open(std::move(token)))
				return false;
		}

		// Is the current token a closing brace?
		else if(braceType == BraceType::Closing)
		{
			if(!braces.close(std::move(token)))
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
				if(!createFunction(tokens))
					return false;
			}

			// "type" indicates that a type should be created.
			else if(token == "type")
			{
				// If the type creation fails, stop parsing.
				if(!createType(tokens))
					return false;
			}
		}
	}

	// If we're not in a global scope and braces are still
	// being tracked, we have unterminated braces.
	if(parent && braces.depth() > 0)
	{
		printf("Unterminated brace '%c'\n", braces.getMostRecent()[0]);
		return false;
	}

	return true;
}

}
