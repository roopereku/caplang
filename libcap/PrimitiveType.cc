#include <cap/PrimitiveType.hh>

namespace cap
{

PrimitiveType::PrimitiveType(std::string_view name, size_t size) :
	Type(Scope::getSharedScope(), Token(Token::Type::Identifier, name, 0, 0)), size(size)
{
}

bool PrimitiveType::isValid()
{
	return size > 0;
}

bool PrimitiveType::hasOperator(TwoSidedOperator::Type type)
{
	return true;
}

bool PrimitiveType::hasOperator(OneSidedOperator::Type type)
{
	return true;
}

}
