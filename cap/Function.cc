#include <cap/Function.hh>
#include <cap/Source.hh>
#include <cap/Client.hh>

#include <cassert>

namespace cap
{

Function::Function()
	: Declaration(Type::Function)
{
}

std::weak_ptr <Node> Function::handleToken(ParserContext& ctx, Token& token)
{
	if(name.empty())
	{
		// TODO: Check if there is no name but the token is an
		// opening parenthesis, go straight to signature parsing.
		
		if(token.getType() != Token::Type::Identifier)
		{
			SourceLocation location(ctx.source, token);
			ctx.client.sourceError(location, "Expected an identifier after 'func'");
			return {};
		}

		setToken(token);
		name = ctx.source.getString(token);
		return weak_from_this();
	}

	else if(!signature)
	{
		if(!token.isOpeningBracket(ctx, '('))
		{
			// TODO: How about anonymous functions?
			SourceLocation location(ctx.source, token);
			ctx.client.sourceError(location, "Expected '(' after function name");
			return {};
		}

		signature = std::make_shared <Expression::Root> ();
		adopt(signature);

		// Delegate the opening bracket to the expression.
		return signature->handleToken(ctx, token);
	}

	else if(!body)
	{
		// Expect a scope beginning.
		if(!token.isOpeningBracket(ctx, '{'))
		{
			SourceLocation location(ctx.source, token);
			ctx.client.sourceError(location, "Expected '{' after a function declaration");
			return {};
		}

		body = std::make_shared <Scope> (false);
		adopt(body);

		return body;
	}

	// Return to the parent node upon a closing brace.
	if(token.isClosingBracket(ctx, '}'))
	{
		assert(!getParent().expired());
		return getParent();
	}

	assert(false);
	return {};
}

std::shared_ptr <Scope> Function::getBody()
{
	return body;
}

}
