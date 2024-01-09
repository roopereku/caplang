#ifndef CAP_VALIDATOR_HH
#define CAP_VALIDATOR_HH

#include <cap/Node.hh>
#include <cap/EventEmitter.hh>

#include <cap/node/Operator.hh>
#include <cap/node/Value.hh>
#include <cap/node/ExpressionRoot.hh>
#include <cap/node/ScopeDefinition.hh>
#include <cap/node/TypeDefinition.hh>
#include <cap/node/InitializationRoot.hh>
#include <cap/node/VariableDefinition.hh>

#include <deque>

namespace cap
{

class TwoSidedOperator;

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

	/// Recursively resolves the result of an access operator.
	std::shared_ptr <Node> resolveAccess(std::shared_ptr <TwoSidedOperator> node);

	std::shared_ptr <ScopeDefinition> getCurrentScope(std::shared_ptr <Node> root);

	/// Locates a definition within the given context with the name pointed at by a value node.
	std::shared_ptr <Node> getDefinition(std::shared_ptr <Value> node,
										std::shared_ptr <ScopeDefinition> context);

	std::shared_ptr <TypeDefinition> getDefinitionType(std::shared_ptr <Node> definition);

	/// Holds unvalidated definitions that are being used in an expression.
	std::deque <std::shared_ptr <Node>> inValidation;
	EventEmitter& events;
};

}

#endif