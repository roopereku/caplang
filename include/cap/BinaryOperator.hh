#ifndef CAP_BINARY_OPERATOR_HH
#define CAP_BINARY_OPERATOR_HH

#include <cap/Expression.hh>
#include <cap/ArgumentAccessor.hh>

namespace cap
{

class BinaryOperator : public Expression
{
public:
	enum class Type
	{
		Assign,
		Access,
		Comma,

		Add,
		Subtract,
		Multiply,
		Divide,
		Modulus,
		Exponent,

		Or,
		And,
		Less,
		Greater,
		LessEqual,
		GreaterEqual,
		Equal,
		Inequal,

		BitwiseShiftLeft,
		BitwiseShiftRight,
		BitwiseAnd,
		BitwiseOr,
		BitwiseXor
	};

	BinaryOperator(Type type, bool compound = false)
		: Expression(Expression::Type::BinaryOperator), type(type), compound(compound)
	{
	}

	/// Creates a binary operator based on the given token.
	///
	/// \param ctx The parsing context to get the source from.
	/// \param token The token to parse a binary operator from.
	/// \return New binary operator or null.
	static std::shared_ptr <BinaryOperator> create(cap::ParserContext& ctx, Token token);

	/// Sets the left hand side and right hand side values.
	///
	/// \param node The left hand side or the right hand side value.
	void handleValue(std::shared_ptr <Expression> node) override;

	/// Checks whether this operator has values on both sides.
	///
	/// \return True if this operator has values on both sides.
	bool isComplete() const override;

	/// Returns the precedence value for the held binary operator type.
	///
	/// \return Something between binaryPrecedenceStart and binaryPrecendenceEnd or binaryAccessPrecedence
	unsigned getPrecedence() override;

	/// Gets the type of this binary operator.
	///
	/// \return The type of this binary operator.
	Type getType();

	std::shared_ptr <Expression> getLeft();
	std::shared_ptr <Expression> getRight();

	static const char* getTypeString(Type type);
	const char* getTypeString() const override;

	class ArgumentAccessor;

protected:
	std::shared_ptr <Expression> stealLatestValue() override;

private:
	std::shared_ptr <Expression> left;
	std::shared_ptr <Expression> right;

	Type type;
	bool compound;
};

class BinaryOperator::ArgumentAccessor : public cap::ArgumentAccessor
{
public:
	ArgumentAccessor(std::shared_ptr <BinaryOperator> root);

	/// Gets the left and right hand side expressions sequentially.
	///
	/// \return The binary operator operands sequentially.
	std::shared_ptr <Expression> getNext() override;

private:
	std::shared_ptr <BinaryOperator> op;
	std::shared_ptr <Expression> current;
};

}

#endif
