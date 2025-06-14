#include <cap/Value.hh>
#include <cap/Identifier.hh>
#include <cap/Integer.hh>
#include <cap/String.hh>
#include <cap/ParserContext.hh>
#include <cap/Client.hh>
#include <cap/Source.hh>

#include <cassert>

namespace cap
{

Value::Value(Type type)
	: Expression(Expression::Type::Value), type(type)
{
}

std::shared_ptr <Value> Value::create(ParserContext& ctx, Token& token)
{
	switch(token.getType())
	{
		case Token::Type::Hexadecimal:
		case Token::Type::Binary:
		case Token::Type::Octal:
		case Token::Type::Integer:
		{
			return Integer::parse(ctx, token);
		}

		// TODO: Include character?
		case Token::Type::String:
		{
			return std::make_shared <String> (ctx.source.getString(token));
		}

		case Token::Type::Identifier:
		{
			return std::make_shared <Identifier> (ctx.source.getString(token));
		}

		// TODO: Parse arrays and tuples from opening brackets here?
		default:
		{
			SourceLocation location(ctx.source, token);
			ctx.client.sourceError(location, "FIXME: Unable to construct a value from '", ctx.source.getString(token), "'");
		}
	}

	return nullptr;
}

bool Value::isComplete() const
{
	assert(false && "cap::Value::isComplete should never be called");
	return false;
}

Value::Type Value::getType() const
{
	return type;
}

}
