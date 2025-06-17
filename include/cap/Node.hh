#ifndef CAP_NODE_HH
#define CAP_NODE_HH

#include <cap/Token.hh>

#include <memory>

namespace cap
{

class Scope;
class Function;
class ParserContext;
class DeclarationStorage;

class Node : public std::enable_shared_from_this <Node>
{
public:
	enum class Type
	{
		Scope,
		Expression,
		Declaration,
		Statement,
		Custom
	};

	class ParserContext;

	/// Implementation defined behavior for handling a token.
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return The new "current node".
	virtual std::weak_ptr <Node> handleToken(Node::ParserContext& ctx, Token& token);

	/// Implementation defined behavior for determining a new "current node" when
	/// the parsing of a node invoked by this node is finished.
	///
	/// \param ctx The parser context containing the exiting node.
	/// \param token The token that caused the exit.
	/// \return The new "current node".
	virtual std::weak_ptr <Node> invokedNodeExited(Node::ParserContext& ctx, Token& token);

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

	/// Sets the token associated with this node.
	///
	/// \param token The token to associate with this node.
	void setToken(Token token);

	/// Gets the token associated with this node.
	///
	/// \return The token associated with this node.
	Token getToken();

	/// Find the nearest parent scope relative to this node.
	///
	/// \return The nearest parent scope up until the global scope or null.
	std::shared_ptr <Scope> getParentScope() const;

	/// Find the nearest parent function relative to this node.
	///
	/// \return The nearest parent function or null if not inside a function.
	std::shared_ptr <Function> getParentFunction() const;

	/// Find the nearest parent node with a valid declaration storage.
	///
	/// \return The nearest node with a valid declaration storage or null.
	std::shared_ptr <Node> getParentWithDeclarationStorage() const;

	/// Find the nearest declaration storage contained by a parent node.
	///
	/// \return The nearest declaration storage.
	DeclarationStorage& getParentDeclarationStorage();

	/// Gets the declaration storage associated with this node.
	///
	/// \return The declaration storage associated with this node.
	DeclarationStorage& getDeclarationStorage();

	// TODO: DeclarationStorage should probably be a pointer or an optional.

	virtual const char* getTypeString() const = 0;

protected:
	Node(Type type);
	Node(Type type, DeclarationStorage& declStorage);

private:
	std::shared_ptr <Node> findParentNode(bool (*filter)(std::shared_ptr <Node>)) const;

	Type type;
	DeclarationStorage& declStorage;
	std::weak_ptr <Node> parent;
	Token at;
};

}

#endif
