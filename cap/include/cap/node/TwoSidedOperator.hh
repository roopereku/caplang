#ifndef CAP_NODE_TWO_SIDED_OPERATOR_HH
#define CAP_NODE_TWO_SIDED_OPERATOR_HH

#include <cap/node/Operator.hh>

namespace cap
{

class TwoSidedOperator : public Operator
{
public:
	enum class Type
	{
		Assignment,
		Addition,
		Subtraction,
		Multiplication,
		Division,
		Exponent,
		Modulus,
		Access,
		Comma,

		GreaterThan,
		GreaterOrEqual,
		LessThan,
		LessOrEqual,
		Equals,
		NotEquals,
		Or,
		And,

		BitwiseShiftLeft,
		BitwiseShiftRight,
		BitwiseAnd,
		BitwiseOr,
		BitwiseXor,
	};

	TwoSidedOperator(Token token, Type type)
		: Operator(Operator::Type::TwoSided, token), type(type)
	{
	}

	const char* getTypeString() override;
	unsigned getPrecedence() override;

	void setLeft(std::shared_ptr <Expression> value)
	{
		left = std::move(value);
	}

	void setRight(std::shared_ptr <Expression> value)
	{
		right = std::move(value);
	}

	std::shared_ptr <Expression> getLeft()
	{
		return left;
	}

	std::shared_ptr <Expression> getRight()
	{
		return right;
	}

	/// Sets the left and right expressions.
	///
	/// \param node The expression node to set as left or right.
	/// \return True if succesful.
	bool handleValue(std::shared_ptr <Expression>&& node) override;

	/// Parses a two sided operator from the given token.
	///
	/// \param token The token to parse a two sided operator from.
	/// \returns New two sided operator or nullptr if the token has an invalid operator.
	static std::shared_ptr <Operator> parseToken(Token& token);

	/// Steals the most recently added value.
	///
	/// \return The most recently added value.
	std::shared_ptr <Expression> stealMostRecentValue() override;

	/// Checks if left and right are set.
	///
	/// \return True if left and right are set.
	bool isComplete() override;

	const Type type;

protected:
	std::shared_ptr <Expression> left;
	std::shared_ptr <Expression> right;
};

}

#endif
