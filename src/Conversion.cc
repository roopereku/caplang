#include <cap/Conversion.hh>
#include <cap/ParserContext.hh>

namespace cap
{

std::weak_ptr <Node> Conversion::handleToken(ParserContext& ctx, Token& token)
{
	if(!context)
	{
		context = std::make_shared <Expression::Root> ();
		adopt(context);
		return context->handleToken(ctx, token);
	}

	return startParsingBody(ctx, token);
}

std::weak_ptr <Node> Conversion::invokedNodeExited(ParserContext& ctx, Token& token)
{
	if(ctx.exitedFrom == context)
	{
		// TODO: Determine the type of the conversion based on what the context contains.
		return weak_from_this();
	}

	return Function::invokedNodeExited(ctx, token);
}

const char* Conversion::getTypeString() const
{
	return "Conversion";
}

}
