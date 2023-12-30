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

	bool handleValue(std::shared_ptr <Expression>&& value) override;

	/// Parses a two sided operator from the given token.
	///
	/// \param token The token to parse a two sided operator from.
	/// \returns New two sided operator or nullptr if the token has an invalid operator.
	static std::shared_ptr <Operator> parseToken(Token& token);

	const Type type;

	friend class OneSidedOperator;

protected:
	/// Replaces the rhs node with the given operator.
	bool replaceExpression(std::shared_ptr <Expression> node) override;

	bool handleHigherPrecedence(std::shared_ptr <Operator> op) override;

	std::shared_ptr <Expression> left;
	std::shared_ptr <Expression> right;
};

}

#endif
