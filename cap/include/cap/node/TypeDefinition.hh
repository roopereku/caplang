#ifndef CAP_NODE_TYPE_DEFINITION_HH
#define CAP_NODE_TYPE_DEFINITION_HH

#include <cap/node/ScopeDefinition.hh>

namespace cap
{

class TypeDefinition : public ScopeDefinition
{
public:
	TypeDefinition(Token name) : ScopeDefinition(Type::TypeDefinition, name)
	{
	}

	TypeDefinition(Token name, size_t size)
		: ScopeDefinition(Type::TypeDefinition, name), size(size)
	{
	}

	static std::shared_ptr <TypeDefinition> getPrimitive(Token token);

	/// Ensures that primitive types are adopted by the shared scope.
	static void ensurePrimitivesAdopted();

private:
	size_t size = 0;
};

}

#endif
