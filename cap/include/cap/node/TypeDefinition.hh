#ifndef CAP_NODE_TYPE_DEFINITION_HH
#define CAP_NODE_TYPE_DEFINITION_HH

#include <cap/node/ScopeDefinition.hh>

namespace cap
{

class TypeDefinition : public ScopeDefinition
{
public:
	enum class Type
	{
		FunctionSignature,
		PrimitiveType,
		CustomType
	};

	/// Initializes a TypeDefinition with the given type.
	/// \param name The type name.
	/// \param type The type definition type.
	TypeDefinition(Token name, Type type)
		: ScopeDefinition(ScopeDefinition::Type::TypeDefinition, name), type(type)
	{
	}

	/// Initializes a TypeDefinition using CustomType.
	/// \param name The type name.
	/// \param type The type definition type.
	TypeDefinition(Token name)
		: ScopeDefinition(ScopeDefinition::Type::TypeDefinition, name), type(Type::CustomType)
	{
	}

private:
	Type type;
};

}

#endif
