#ifndef CAP_NODE_ONE_SIDED_OPERATOR_HH
#define CAP_NODE_ONE_SIDED_OPERATOR_HH

#include <cap/node/Operator.hh>

namespace cap
{

class OneSidedOperator : public Operator
{
public:
	enum class Type
	{
		Call,
		Subscript,

		Not,
		Negate,
		BitwiseNot,
	};

	OneSidedOperator(Token token, Type type)
		: Operator(Operator::Type::OneSided, token), type(type)
	{
	}

	const char* getTypeString() override;
	unsigned getPrecedence() override;

	bool affectsPreviousValue();
	bool affectsNextValue();

	std::shared_ptr <Expression> getExpression()
	{
		return expression;
	}

	void setExpression(std::shared_ptr <Expression> value)
	{
		expression = std::move(value);
	}

	/// Saves the given expression node as the expression.
	bool handleValue(std::shared_ptr <Expression>&& node) override;

	/// Parses a one sided operator from the given token.
	///
	/// \param token The token to parse a one sided operator from.
	/// \returns New one sided operator or nullptr if the token has an invalid operator.
	static std::shared_ptr <Operator> parseToken(Token& token);

	/// Steals the held expression.
	///
	/// \return The held expression.
	std::shared_ptr <Expression> stealMostRecentValue() override;

	/// Checks if the expression is set.
	///
	/// \return True if the expression is set.
	bool isComplete() override;

	const Type type;

	friend class TwoSidedOperator;

protected:
	std::shared_ptr <Expression> expression;
};

}

#endif
