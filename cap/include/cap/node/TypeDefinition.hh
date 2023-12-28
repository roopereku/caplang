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
};

}

#endif
