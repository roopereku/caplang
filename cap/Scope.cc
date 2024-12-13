#include <cap/Scope.hh>
#include <cap/Client.hh>
#include <cap/Function.hh>

#include <cassert>

namespace cap
{

Scope::Scope()
	: requiresBrackets(false), onlyDeclarations(true)
{
}

Scope::Scope(bool onlyDeclarations)
	: requiresBrackets(true), onlyDeclarations(onlyDeclarations)
{
}

std::weak_ptr <Node> Scope::handleToken(ParserContext& ctx, Token& token)
{
	if(requiresBrackets)
	{
		// The first token of a scope isolated by
		// by brackets requires an opener.
		if(ctx.tokensProcessed == 0)
		{
			if(!token.isOpeningBracket(ctx, '{'))
			{
				SourceLocation location(ctx.source, token);
				ctx.client.sourceError(location, "Expected '{' to indicate the beginning of a scope");
				return {};
			}
		}

		else if(!token.isOpeningBracket(ctx, '}'))
		{
			assert(!getParent().expired());
			return getParent();
		}
	}

	if(ctx.source.match(token, L"func"))
	{
		return appendNested(std::make_shared <Function> ());
	}

	return weak_from_this();
}

std::weak_ptr <Node> Scope::appendNested(std::shared_ptr <Node>&& node)
{
	adopt(node);

	if(!nested)
	{
		nested = std::move(node);
		return nested->weak_from_this();
	}

	return nested->appendNext(std::move(node));
}

}
