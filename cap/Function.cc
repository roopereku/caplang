#include <cap/Function.hh>
#include <cap/Source.hh>
#include <cap/Client.hh>

#include <cassert>

namespace cap
{

Function::Function()
	: Scope(Type::Function, false)
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

	assert(signature);
	return Scope::handleToken(ctx, token);
}

}
