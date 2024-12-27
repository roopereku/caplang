#include <cap/Function.hh>
#include <cap/Signature.hh>
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

		// The signature has to come after the name.
		name = ctx.source.getString(token);
		signature = std::make_shared <Signature> ();
		adopt(signature);

		return signature;
	}

	assert(signature);
	return Scope::handleToken(ctx, token);
}

}
