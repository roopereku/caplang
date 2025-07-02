#include <cap/Conversion.hh>
#include <cap/ParserContext.hh>
#include <cap/Client.hh>
#include <cap/Source.hh>

#include <cassert>

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
		assert(context->getFirst());

		// The only thing that we care about at this point is the type of the
		// conversion based on the first reachable node within an expression.
		// After that is known, we can rely on the resulting types of operands
		// referring to different type names.
		switch(context->getFirst()->getType())
		{
			case Expression::Type::BinaryOperator:
			{
				// TODO: Add implicit parameters for left and right.

				// TODO: Handle the case where an arrow operator aims to define a
				// type conversion outside a class type.

				type = Type::BinaryOperator;
				break;
			}

			case Expression::Type::UnaryOperator:
			{
				// TODO: Add an implicit parameter for the operand.

				type = Type::UnaryOperator;
				break;
			}

			case Expression::Type::BracketOperator:
			{
				// TODO: Forbid operator <>

				// TODO: Add implicit parameters for the operand and whatever is inside the brackets.

				type = Type::BracketOperator;
				break;
			}

			case Expression::Type::Value:
			{
				// Add an implicit parameter for the operand.

				type = Type::TypeConversion;
				break;
			}

			default:
			{
				// TODO: Give more context on why the conversion is invalid.
				SourceLocation location(ctx.source, token);
				ctx.client.sourceError(location, "Invalid conversion");
				return {};
			}
		}

		return weak_from_this();
	}

	return Function::invokedNodeExited(ctx, token);
}

const char* Conversion::getTypeString() const
{
	return "Conversion";
}

}
