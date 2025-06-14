#include <cap/Integer.hh>
#include <cap/ParserContext.hh>
#include <cap/Source.hh>
#include <cap/Client.hh>

namespace cap
{

Integer::Integer(uint64_t value)
	: Value(Type::Integer), value(value)
{
}

std::shared_ptr <Integer> Integer::parse(ParserContext& ctx, Token& token)
{
	uint64_t result;
	int base;

	switch(token.getType())
	{
		case Token::Type::Hexadecimal: base = 16; break;
		case Token::Type::Binary: base = 2; break;
		case Token::Type::Octal: base = 8; break;
		default: base = 10; break;
	}

	try
	{
		result = std::stoull(ctx.source.getString(token), nullptr, base);
	}

	catch(const std::invalid_argument&)
	{
		// TODO: Should tokenizer or this be responsible of checking for malformed integers?
		SourceLocation location(ctx.source, token);
		ctx.client.sourceError(location, "FIXME: Unable to construct an integer from '", ctx.source.getString(token), "'.");
		return nullptr;
	}

	catch(const std::out_of_range&)
	{
		SourceLocation location(ctx.source, token);
		ctx.client.sourceError(location, "Integer too large to fit inside 64 bits");
		return nullptr;
	}

	return std::make_shared <Integer> (result);
}

uint64_t Integer::getValue() const
{
	return value;
}

void Integer::updateResultType(cap::ParserContext& ctx)
{
	// TODO: Check the initial value and determine a type based on that.
	setResultType(ctx.client.getBuiltin().getDefaultIntegerType());
}

const char* Integer::getTypeString() const
{
	return "Integer";
}

}
