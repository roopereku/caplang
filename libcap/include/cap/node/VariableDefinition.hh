#ifndef CAP_NODE_VARIABLE_DEFINITION_HH
#define CAP_NODE_VARIABLE_DEFINITION_HH

#include <cap/Variable.hh>
#include <cap/node/Declaration.hh>
#include <cap/node/TwoSidedOperator.hh>

namespace cap
{

class VariableDefinition : public Declaration
{
public:
	VariableDefinition(Variable&& variable);

	bool isVariableDefinition() final override
	{
		return true;
	}

	const Token& getName() override
	{
		return variable.getName();
	}

	const Variable& getVariable()
	{
		return variable;
	}

private:
	Variable variable;
};

}

#endif
