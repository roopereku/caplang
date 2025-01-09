#include <cap/TypeDefinition.hh>

namespace cap
{

TypeDefinition::TypeDefinition(Type type)
	: Declaration(Declaration::Type::TypeDefinition), type(type)
{
}

TypeDefinition::Type TypeDefinition::getType()
{
	return type;
}

}
