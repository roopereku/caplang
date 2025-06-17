#ifndef CAP_UNARY_OPERATOR_HH
#define CAP_UNARY_OPERATOR_HH

#include <cap/Expression.hh>

namespace cap
{

class UnaryOperator : public Expression
{
public:
	enum class Type
	{
		Negate,
		LogicalNot,
		BitwiseNot,
		ParseTime,

		PreIncrement,
		PreDecrement,
		PostIncrement,
		PostDecrement,
	};

	UnaryOperator(Type type)
		: Expression(Expression::Type::UnaryOperator), type(type)
	{
	}

	/// Creates a prefix unary operator based on the given token.
	///
	/// \param ctx The parsing context to get the source from.
	/// \param token The token to parse a unary operator from.
	/// \return New unary operator or null.
	static std::shared_ptr <UnaryOperator> createPrefix(cap::ParserContext& ctx, Token token);

	/// Creates a postfix unary operator based on the given token.
	///
	/// \param ctx The parsing context to get the source from.
	/// \param token The token to parse a unary operator from.
	/// \return New unary operator or null.
	static std::shared_ptr <UnaryOperator> createPostfix(cap::ParserContext& ctx, Token token);

	/// Sets the left hand side and right hand side values.
	///
	/// \param node The left hand side or the right hand side value.
	void handleValue(std::shared_ptr <Expression> node) override;

	/// Checks whether this operator has values on both sides.
	///
	/// \return True if this operator has values on both sides.
	bool isComplete() const override;

	/// Returns precedence values above 0.
	///
	/// \return Value above 0 depending on the type.
	unsigned getPrecedence() override;

	/// Gets the type of this unary operator.
	///
	/// \return The type of this unary operator.
	Type getType();

	std::shared_ptr <Expression> getExpression();

	static const char* getTypeString(Type type);
	const char* getTypeString() const override;

protected:
	std::shared_ptr <Expression> stealLatestValue() override;

private:
	std::shared_ptr <Expression> expression;
	Type type;
};

}

#endif
