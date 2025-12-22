#ifndef CAP_ATTRIBUTE_HH
#define CAP_ATTRIBUTE_HH

#include <cap/Expression.hh>
#include <cap/Declaration.hh>

namespace cap
{

class Attribute : public Declaration
{
public:
	Attribute();

	/// Creates the initializer expression and delegates the token forward.
	///
	/// \param ctx The parser context.
	/// \param token The token to handle.
	/// \return The initializer expression.
	std::weak_ptr <Node> handleToken(Node::ParserContext& ctx, Token& token) override;

	const char* getTypeString() const override;
	bool validate(Validator& validator) override;

	class Root;
};

class Attribute::Root : public Expression::Root
{
public:
	Root();

	unsigned getPrecedence() override;

	const char* getTypeString() const override;
};

}

#endif
