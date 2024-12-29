#ifndef CAP_NODE_HH
#define CAP_NODE_HH

#include <cap/ParserContext.hh>
#include <cap/Token.hh>

#include <memory>

namespace cap
{

class Node : public std::enable_shared_from_this <Node>
{
public:
	enum class Type
	{
		Scope,
		Expression,
		Custom
	};

	class ParserContext;
	class Traverser;

	/// Implementation defined behavior for handling a token.
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return The new "current node".
	virtual std::weak_ptr <Node> handleToken(Node::ParserContext& ctx, Token& token);

	/// Gets the parent node.
	///
	/// \return The parent node.
	std::weak_ptr <Node> getParent() const;

	/// Adopts the given node. This only sets the
	/// parent of given node.
	///
	/// \param node The node to adopt.
	void adopt(std::shared_ptr <Node> node);

	/// Gets the the type of this node.
	///
	/// \return The type of this node.
	Type getType();

protected:
	Node(Type type);

private:
	std::weak_ptr <Node> parent;
	Type type;
};

class Node::ParserContext : public cap::ParserContext
{
public:
	ParserContext(Client& client, Source& source)
		: cap::ParserContext(client, source)
	{
	}

	/// How many tokens have been processed since the node was
	/// switched.
	size_t tokensProcessed = 0;

	/// How many nested subexpressions are there currently?
	int subExpressionDepth = 0;

	/// Used to stop the recursive openings of subexpressions.
	/// Such could happen when a switch to the parent
	/// expression node is made during handleToken.
	bool canOpenSubexpression = true;
};

class Scope;
class Expression;
class Class;
class Function;
class ClassType;

class Node::Traverser
{
public:
	/// Performs depth first traversal on a node.
	///
	/// \param node The node to traverse.
	/// \return True if traversing should be continued.
	bool traverseNode(std::shared_ptr <Node> node);

	/// Performs depth first traversal on a scope.
	///
	/// \param node The scope to traverse.
	/// \return True if traversing should be continued.
	bool traverseScope(std::shared_ptr <Scope> node);

	/// Performs depth first traversal on an expression.
	///
	/// \param node The expression to traverse.
	/// \return True if traversing should be continued.
	bool traverseExpression(std::shared_ptr <Expression> node);

protected:
	// TODO: Add onNodeExited?

	/// Invoked upon hitting a custom node.
	///
	/// \param node The node representing a custom node.
	void onCustomNode(std::shared_ptr <Node> node);

	/// Invoked upon hitting a standalone scope node.
	///
	/// \param node The node representing a scope.
	/// \return True if the traversing should extend to the nested nodes.
	virtual bool onScope(std::shared_ptr <Scope> node);

	/// Invoked upon hitting a function node.
	///
	/// \param node The node representing a function.
	/// \return True if the traversing should extend to the nested nodes.
	virtual bool onFunction(std::shared_ptr <Function> node);

	/// Invoked upon hitting a class type.
	///
	/// \param node The node representing a class type.
	/// \return True if the traversing should extend to the nested nodes.
	virtual bool onClassType(std::shared_ptr <ClassType> node);

	/// Invoked upon hitting a custom scope.
	///
	/// \param node The node rerpresenting a custom scope.
	/// \return True if the traversing should extend to the nested nodes.
	virtual bool onCustomScope(std::shared_ptr <Scope> node);

	/// Invoked upon hitting an expression node.
	///
	/// \param node The node representing an expression.
	virtual void onExpression(std::shared_ptr <Expression> node);
};

}

#endif
