#ifndef CAP_NODE_VARIABLE_DEFINITION_HH
#define CAP_NODE_VARIABLE_DEFINITION_HH

#include <cap/node/ExpressionRoot.hh>

namespace cap
{

class VariableDefinition : public ExpressionRoot
{
public:
	VariableDefinition(Token& token)
		: ExpressionRoot(ExpressionRoot::Type::VariableDefinition, token)
	{
	}
};

}

#endif
