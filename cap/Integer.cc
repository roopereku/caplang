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

	try
	{
		result = std::stoull(ctx.source.getString(token));
	}

	catch(const std::invalid_argument&)
	{
		// Tokenizer should be able to validate whether integers are valid.
		SourceLocation location(ctx.source, token);
		ctx.client.sourceError(location, "FIXME: Unable to construct an integer from '", ctx.source.getString(token), "'.");
		return nullptr;
	}

	catch(const std::out_of_range&)
	{
		SourceLocation location(ctx.source, token);
		ctx.client.sourceError(location, "Integer '", ctx.source.getString(token), "' too large to fit inside 64 bits");
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
