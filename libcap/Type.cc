#include <cap/Type.hh>
#include <cap/BraceMatcher.hh>
#include <cap/PrimitiveType.hh>

namespace cap
{

static PrimitiveType primitives[]
{
	PrimitiveType("uint8", 1),
	PrimitiveType("uint16", 2),
	PrimitiveType("uint32", 4),
	PrimitiveType("uint64", 8),

	PrimitiveType("int8", 1),
	PrimitiveType("int16", 2),
	PrimitiveType("int32", 4),
	PrimitiveType("int64", 8),

	PrimitiveType("invalidType", 0),
};

enum class DefaultPrimitiveIndex
{
	// uint64
	Unsigned = 3,

	// int64
	Signed = 7,

	// invalidType
	Invalid = 8
};

bool Type::parse(ParserState& state)
{
	printf("Parsing type '%s'\n", name.getString().c_str());

	// Make sure that the next token is an opening curly brace.
	Token signatureOpener = state.tokens.next();
	if(signatureOpener.getType() != Token::Type::CurlyBrace || signatureOpener[0] != '{')
	{
		printf("Expected '{' after a type name\n");
		return false;
	}

	// Open the type body.
	state.braces.open(std::move(signatureOpener));

	// Parse the type body and stop on failure.
	printf("Parsing body of '%s'\n", name.getString().c_str());
	if(!Scope::parse(state))
		return false;

	return true;
}

PrimitiveType& Type::getPrimitive(Token::Type tokenType)
{
	switch(tokenType)
	{
		case Token::Type::Integer:
			return primitives[static_cast <size_t> (DefaultPrimitiveIndex::Signed)];

		default:
			return getInvalid();
	}
}

PrimitiveType& Type::getInvalid()
{
	return primitives[static_cast <size_t> (DefaultPrimitiveIndex::Invalid)];
}

bool Type::hasOperator(TwoSidedOperator::Type type)
{
	return false;
}

bool Type::hasOperator(OneSidedOperator::Type type)
{
	return false;
}

}
