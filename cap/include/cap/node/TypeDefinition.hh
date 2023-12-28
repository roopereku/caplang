#ifndef CAP_NODE_TYPE_DEFINITION_HH
#define CAP_NODE_TYPE_DEFINITION_HH

#include <cap/node/ScopeDefinition.hh>

namespace cap
{

class TypeDefinition : public ScopeDefinition
{
public:
	TypeDefinition() : ScopeDefinition(Type::TypeDefinition)
	{
	}
};

}

#endif
