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
		Declaration,
		Custom
	};

	class ParserContext;

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

	/// Sets the token associated with this node.
	///
	/// \param token The token to associate with this node.
	void setToken(Token token);

	/// Gets the token associated with this node.
	///
	/// \return The token associated with this node.
	Token getToken();

	virtual const char* getTypeString() = 0;

protected:
	Node(Type type);

private:
	std::weak_ptr <Node> parent;
	Type type;
	Token at;
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

}

#endif
