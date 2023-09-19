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
		Multiplication,
		Division,
	};

	TwoSidedOperator(Token&& token, Type type)
		: Operator(std::move(token)), type(type)
	{
	}

	bool applyCached(std::shared_ptr <Expression>&& cached) override;

	const char* getTypeString() override;
	unsigned getPrecedence() override;

	bool isTwoSided() override;

	std::shared_ptr <Expression> getLeft()
	{
		return left;
	}

	std::shared_ptr <Expression> getRight()
	{
		return right;
	}

protected:
	/// Replaces the rhs node with the given operator.
	bool replaceExpression(std::shared_ptr <Expression> node) override;

	bool handleLowerPrecedence(std::shared_ptr <Operator> op, ParserState& state) override;
	bool handleHigherPrecedence(std::shared_ptr <Operator> op, ParserState& state) override;
	bool handleValue(std::shared_ptr <Value> value, ParserState& state) override;

	std::shared_ptr <Expression> left;
	std::shared_ptr <Expression> right;

	Type type;
};

}

#endif
