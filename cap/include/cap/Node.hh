#ifndef CAP_NODE_HH
#define CAP_NODE_HH

#include <cap/ParserContext.hh>
#include <cap/Token.hh>

#include <memory>

namespace cap
{

class SourceLocation;

class Node : public std::enable_shared_from_this <Node>
{
public:
	class ParserContext;

	/// Implementation defined behavior for handling a token.
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return The new "current node".
	virtual std::weak_ptr <Node> handleToken(Node::ParserContext& ctx, Token& token);

	/// Initializes the next node if unset. Otherwise recursively calls
	/// this function for the next node.
	///
	/// \param node The node to append as the next one.
	/// \return Weak reference to the appended node.
	std::weak_ptr <Node> appendNext(std::shared_ptr <Node>&& node);

	/// Gets the next node.
	///
	/// \return The next node.
	std::weak_ptr <Node> getNext() const;

	/// Gets the parent node.
	///
	/// \return The parent node.
	std::weak_ptr <Node> getParent() const;

	/// Adopts the given node. This only sets the
	/// parent of given node.
	///
	/// \param node The node to adopt.
	void adopt(std::shared_ptr <Node> node);

private:
	std::weak_ptr <Node> parent;
	std::shared_ptr <Node> next;
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
};

}

#endif
