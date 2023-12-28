#ifndef CAP_NODE_FUNCTION_DEFINITION_HH
#define CAP_NODE_FUNCTION_DEFINITION_HH

#include <cap/node/ScopeDefinition.hh>

namespace cap
{

class FunctionDefinition : public ScopeDefinition
{
public:
	FunctionDefinition(Token name) : ScopeDefinition(Type::FunctionDefinition, name)
	{
	}
};

}

#endif
