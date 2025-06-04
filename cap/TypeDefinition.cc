#include <cap/TypeDefinition.hh>

namespace cap
{

TypeDefinition::TypeDefinition(Type type)
	: type(type)
{
}

TypeDefinition::Type TypeDefinition::getType()
{
	return type;
}

}
