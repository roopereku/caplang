#ifndef CAP_VALIDATOR_HH
#define CAP_VALIDATOR_HH

#include <cap/Node.hh>
#include <cap/Reference.hh>
#include <cap/EventEmitter.hh>

#include <cap/node/Operator.hh>
#include <cap/node/Value.hh>
#include <cap/node/ExpressionRoot.hh>
#include <cap/node/ScopeDefinition.hh>
#include <cap/node/TypeDefinition.hh>
#include <cap/node/ReturnStatement.hh>
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

	// TODO: Make this take a SourceFile parameter instead of a node.
	bool validate(std::shared_ptr <Node> root);

	/// Resolves a definition pointed at by node.
	///
	/// \param node The expression pointing to a definition.
	/// \return The resolved definition.
	Reference resolveDefinition(std::shared_ptr <Expression> node);

	/// Gets the type of a definition.
	///
	/// \param reference The definition.
	/// \return The type of the given definition if any.
	std::shared_ptr <TypeDefinition> getDefinitionType(Reference reference);

	EventEmitter& events;

	// Validates the given node.
	//
	// \param The node to validate.
	// \return True if succesful.
	bool validateNode(std::shared_ptr <Node> node);

private:
	bool validateExpression(std::shared_ptr <Expression> node);
	bool validateExpressionRoot(std::shared_ptr <ExpressionRoot> node);
	bool validateReturn(std::shared_ptr <ReturnStatement> node, bool allowVoid);
	bool validateOperator(std::shared_ptr <Operator> node);
	bool validateScope(std::shared_ptr <ScopeDefinition> node);

	bool checkNameCollision(Token name, std::shared_ptr <Node> context);

	/// Recursively resolves the result of an access operator.
	Reference resolveAccess(std::shared_ptr <TwoSidedOperator> node);

	std::shared_ptr <ScopeDefinition> getCurrentScope(std::shared_ptr <Node> root);
	std::shared_ptr <ScopeDefinition> getParentScope(std::shared_ptr <Node> root);
	std::shared_ptr <ScopeDefinition> getCurrentNamedScope(std::shared_ptr <Node> root);

	/// Locates a definition within the given context with the name pointed at by a value node.
	Reference getDefinition(Token name, std::shared_ptr <ScopeDefinition> context);

	/// Holds unvalidated definitions that are being used in an expression.
	std::deque <std::shared_ptr <Node>> inValidation;
};

}

#endif
