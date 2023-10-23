#include <cap/PrimitiveType.hh>

namespace cap
{

PrimitiveType::PrimitiveType(Scope& parent, std::string_view name, size_t size) :
	Type(parent, Token(Token::Type::Identifier, name, 0, 0)), size(size)
{
}

bool PrimitiveType::hasOperator(TwoSidedOperator::Type type)
{
	printf("CHECK PRIMITIVE\n");
	return true;
}

bool PrimitiveType::hasOperator(OneSidedOperator::Type type)
{
	return true;
}

void PrimitiveType::registerBuiltins(Scope& to)
{
	// TODO: Once type aliases exist, give aliases to some of these types.

	to.createPrimitiveType("int8", 1);
	to.createPrimitiveType("int16", 2);
	to.createPrimitiveType("int32", 3);
	to.createPrimitiveType("int64", 4);

	to.createPrimitiveType("uint8", 1);
	to.createPrimitiveType("uint16", 2);
	to.createPrimitiveType("uint32", 3);
	to.createPrimitiveType("uint64", 4);

	to.createPrimitiveType("invalidType", 0);
}

}
