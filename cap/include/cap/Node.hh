#ifndef CAP_NODE_HH
#define CAP_NODE_HH

#include <cap/Token.hh>

#include <memory>

namespace cap
{

/// Node is a part of an abstract syntax tree.
class Node : public std::enable_shared_from_this <Node>
{
public:
	enum class Type
	{
		Empty,
		ScopeDefinition,
		Expression
	};

	/// Helper function to convert a node to another node type.
	///
	/// \return The same node as the new type.
	template <typename T>
	std::shared_ptr <T> as()
	{
		return std::static_pointer_cast <T> (shared_from_this());
	}

	Node();
	Node(Type type);
	Node(Type type, Token token);

	std::shared_ptr <Node> findLast();
	void adopt(std::shared_ptr <Node> node);
	std::weak_ptr <Node> getParent() const;

	std::shared_ptr <Node> getNext() const;
	void setNext(std::shared_ptr <Node>&& node);

	/// Determines if this node is a definition of the given name.
	///
	/// \param name The name of the definition.
	/// \return The specific node of the definition.
	virtual std::shared_ptr <Node> isDefinition(Token name);

	virtual const char* getTypeString();

	/// The type of the node.
	const Type type;

	/// The associated token.
	const Token token;

private:
	std::shared_ptr <Node> next;
	std::weak_ptr <Node> parent;
};

}

#endif
