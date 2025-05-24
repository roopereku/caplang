#include <cap/Statement.hh>

namespace cap
{

Statement::Statement(Type type)
	: Node(Node::Type::Statement), type(type)
{
}

Statement::Type Statement::getType() const
{
	return type;
}

/// Determines the continuation of whether an expression will be parsed or not.
///
/// \return This statement or the parent node if there's no further tokens on the same line.
std::weak_ptr <Node> Statement::getContinuation(cap::ParserContext& ctx)
{
	// TODO: In case of "else if" either extend the token to cover both "else" and "if"
	// or just store the token of "if" to detect it being the ending of the current line.

	bool expectsTokens = !getToken().isLastOfLine(ctx);
	if(!onInitialize(ctx, expectsTokens))
	{
		return {};
	}

	// If the token representing the statement is the last one
	// on the current line, don't accept any more tokens for this statement.
	return expectsTokens ? weak_from_this() : getParent();
}

}
