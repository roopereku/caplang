#ifndef CAP_TRAVERSER_HH
#define CAP_TRAVERSER_HH

#include <cap/Node.hh>
#include <cap/Expression.hh>
#include <cap/Variable.hh>

namespace cap
{

class Scope;
class Class;
class Function;
class ClassType;
class CallableType;
class BinaryOperator;
class BracketOperator;
class TypeDefinition;
class ModifierRoot;
class Value;

class Traverser
{
public:
	enum class Result
	{
		/// Was the node handled by the deriving class?
		/// NOTE: This implies continuation.
		NotHandled,

		/// Should the traversal continue to node specific child nodes?
		Continue,

		/// Should the current node be exited?
		Exit,

		/// Should the traversal be stopped?
		Stop,
	};

	/// Performs depth first traversal on a node.
	///
	/// \param node The node to traverse.
	/// \return The next step of traversal.
	/// \return False if the traversal was unsuccessful.
	bool traverseNode(std::shared_ptr <Node> node);

	/// Performs depth first traversal on a scope.
	///
	/// \param node The scope to traverse.
	/// \return False if the traversal was unsuccessful.
	bool traverseScope(std::shared_ptr <Scope> node);

	/// Performs depth first traversal on an expression.
	///
	/// \param node The expression to traverse.
	/// \return False if the traversal was unsuccessful.
	bool traverseExpression(std::shared_ptr <Expression> node);

	/// Performs depth first traversal on a declaration.
	///
	/// \param node The declaration to traverse.
	/// \return False if the traversal was unsuccessful.
	bool traverseDeclaration(std::shared_ptr <Declaration> node);

	/// Performs depth first traversal on a type definition.
	///
	/// \param node The type definition to traverse.
	/// \return False if the traversal was unsuccessful.
	bool traverseTypeDefinition(std::shared_ptr <TypeDefinition> node);

protected:
	/// Invoked when a node is exited.
	/// NOTE: This is only called when the exited node had a custom handler.
	///
	/// \param node The node that is exited out of.
	/// \param result The result of the given node.
	virtual void onNodeExited(std::shared_ptr <Node> node, Result result);

	/// Invoked upon hitting a custom node.
	///
	/// \param node The node representing a custom node.
	/// \return The next step of traversal.
	virtual Result onCustomNode(std::shared_ptr <Node> node);

	/// Invoked upon hitting a scope node.
	///
	/// \param node The node representing a scope.
	/// \return Continue if the traversal should continue to the nested nodes.
	virtual Result onScope(std::shared_ptr <Scope> node);

	/// Invoked upon hitting a function node.
	///
	/// \param node The node representing a function.
	/// \return Continue if the traversal should continue to the nested nodes.
	virtual Result onFunction(std::shared_ptr <Function> node);

	/// Invoked upon hitting a class type.
	///
	/// \param node The node representing a class type.
	/// \return Continue if the traversal should continue to the nested nodes.
	virtual Result onClassType(std::shared_ptr <ClassType> node);

	/// Invoked upon hitting a callable type.
	///
	/// \param node The node representing a callable type.
	/// \return Continue if the traversal should continue to the return type and parameters.
	virtual Result onCallableType(std::shared_ptr <CallableType> node);

	/// Invoked upon hitting an expression root.
	///
	/// \param node The node representing an expression root.
	/// \return Continue if the traversal should continue to the first expression node.
	virtual Result onExpressionRoot(std::shared_ptr <Expression::Root> node);

	/// Invoked upon hitting a declaration root in an expression.
	///
	/// \param node The node representing a declaration root.
	/// \return Continue if the traversal should continue to the underlying expression.
	virtual Result onVariable(std::shared_ptr <Variable> node);

	/// Invoked upon hitting a modifier root in an expression.
	///
	/// \param node The node representing a modifier root.
	/// \return Continue if the traversal should continue to the underlying expression.
	virtual Result onModifierRoot(std::shared_ptr <ModifierRoot> node);

	/// Invoked upon hitting a binary operator in an expression.
	///
	/// \param node The node representing a binary operator.
	/// \return Continue if the traversal should continue to left and right nodes.
	virtual Result onBinaryOperator(std::shared_ptr <BinaryOperator> node);

	/// Invoked upon hitting a bracket operator in an expression.
	///
	/// \param node The node representing a bracket operator.
	/// \return Continue if the traversal should continue to the context and the inner expression.
	virtual Result onBracketOperator(std::shared_ptr <BracketOperator> node);

	/// Invoked upon hitting a value in an expression.
	///
	/// \param node The node representing a value.
	/// \return Stop if the value is invalid.
	virtual Result onValue(std::shared_ptr <Value> node);
};

}

#endif
