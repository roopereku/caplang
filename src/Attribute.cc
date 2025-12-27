#include <cap/Attribute.hh>

namespace cap
{

Attribute::Attribute()
	: Expression::Root(Type::Attribute)
{
}

unsigned Attribute::getPrecedence()
{
	return modifierPrecedence;
}

const char* Attribute::getTypeString() const
{
	return "Attribute";
}

}
