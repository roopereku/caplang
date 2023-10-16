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

	TwoSidedOperator(Token&& token, Type type)
		: Operator(std::move(token)), type(type)
	{
	}

	bool applyCached(std::shared_ptr <Expression>&& cached) override;

	const char* getTypeString() override;
	unsigned getPrecedence() override;

	bool isTwoSided() override;

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

	Type getType()
	{
		return type;
	}

	/// Parses a two sided operator from the given token.
	///
	/// \param token The token to parse a two sided operator from.
	/// \state The current parser state.
	/// \returns New two sided operator or nullptr if the token has an invalid operator.
	static std::shared_ptr <Operator> parse(Token&& token, ParserState& state);

	friend class OneSidedOperator;

protected:
	/// Replaces the rhs node with the given operator.
	bool replaceExpression(std::shared_ptr <Expression> node) override;

	bool handleSamePrecedence(std::shared_ptr <Operator> op, ParserState& state) override;
	bool handleHigherPrecedence(std::shared_ptr <Operator> op, ParserState& state) override;
	bool handleValue(std::shared_ptr <Expression> value, ParserState& state) override;

	std::shared_ptr <Expression> left;
	std::shared_ptr <Expression> right;

	Type type;
};

}

#endif
