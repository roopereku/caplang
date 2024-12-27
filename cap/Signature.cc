#include <cap/Signature.hh>
#include <cap/Client.hh>

#include <cassert>

namespace cap
{

std::weak_ptr <Node> Signature::handleToken(ParserContext& ctx, Token& token)
{
	//if(!firstType)
	//{
	//	if(!token.isOpeningBracket(ctx, '('))
	//	{
	//		SourceLocation location(ctx.source, token);
	//		ctx.client.sourceError(location, "Expected '(' to indicate the start of a signature");
	//		return {};
	//	}

	//	firstType = std::make_shared <TypeReference> ();
	//	//return firstType;
	//}

	/*else*/ if(token.isClosingBracket(ctx, ')'))
	{
		assert(!getParent().expired());
		return getParent();
	}

	//else
	//{
	//	printf("Signature '%ls'\n", ctx.source.getString(token).c_str());
	//}

	return weak_from_this();
}

}
