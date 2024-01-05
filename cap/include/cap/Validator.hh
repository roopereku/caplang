#ifndef CAP_VALIDATOR_HH
#define CAP_VALIDATOR_HH

#include <cap/Node.hh>
#include <cap/EventEmitter.hh>

#include <cap/node/Operator.hh>
#include <cap/node/ExpressionRoot.hh>
#include <cap/node/ScopeDefinition.hh>
#include <cap/node/TypeDefinition.hh>

namespace cap
{

/// Validator validates an AST and fills in missing information.
class Validator
{
public:
	Validator(EventEmitter& events);

	bool validate(std::shared_ptr <Node> root);

private:
	bool validateNode(std::shared_ptr <Node> node);
	bool validateExpression(std::shared_ptr <Expression> node);
	bool validateExpressionRoot(std::shared_ptr <ExpressionRoot> node);
	bool validateOperator(std::shared_ptr <Operator> node);
	bool validateScope(std::shared_ptr <ScopeDefinition> node);

	bool validateVariableInit(std::shared_ptr <Expression> node);

	std::shared_ptr <Expression> getLeftmostExpression(std::shared_ptr <Expression> node);

	std::shared_ptr <Node> currentNode;

	EventEmitter& events;
};

}

#endif
