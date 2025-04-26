#include <cap/TypeDefinition.hh>

namespace cap
{

TypeDefinition::TypeDefinition(Type type)
	: Declaration(Declaration::Type::TypeDefinition), type(type)
{
}

TypeDefinition::TypeDefinition(Type type, DeclarationStorage& declStorage)
	: Declaration(Declaration::Type::TypeDefinition, declStorage), type(type)
{
}

TypeDefinition::Type TypeDefinition::getType()
{
	return type;
}

}
