#ifndef CAP_RETURN_HH
#define CAP_RETURN_HH

#include <cap/Statement.hh>
#include <cap/Expression.hh>

namespace cap
{

class Return : public Statement
{
public:
	Return();

	/// Creates the return expression and delegates the token forward.
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return The return expression or null if not inside a proper node.
	std::weak_ptr <Node> handleToken(Node::ParserContext& ctx, Token& token) override;

	/// Exits out of the return statement.
	///
	/// \param ctx The parser context containing the exiting node.
	/// \param token The token that caused the exit.
	/// \return The new parent node.
	std::weak_ptr <Node> invokedNodeExited(Node::ParserContext& ctx, Token& token) override;

	/// Gets the root for the return expression.
	///
	/// \return The root for the return expression
	std::shared_ptr <Expression::Root> getExpression() const;

	/// Gets the node that is returned from.
	///
	/// \return The node that is returned from.
	std::shared_ptr <Node> getReturnedFrom() const;
	
	const char* getTypeString() const override;

private:
	std::shared_ptr <Expression::Root> expression;
};

}

#endif
